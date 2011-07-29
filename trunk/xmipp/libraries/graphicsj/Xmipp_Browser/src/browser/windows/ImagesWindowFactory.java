/*
 * To change this template, choose Tools | Templates
 * and openTableFileImageItem the template in the editor.
 */
package browser.windows;

import browser.DEBUG;
import browser.imageitems.ImageConverter;
import browser.imageitems.tableitems.AbstractTableImageItem;
import browser.table.models.AbstractXmippTableModel;
import ij.IJ;
import ij.ImagePlus;
import browser.table.JFrameImagesTable;
import browser.table.micrographs.JFrameMicrographs;
import browser.table.micrographs.ctf.CTFRecalculateImageWindow;
import browser.table.micrographs.ctf.profile.CTFViewImageWindow;
import browser.table.micrographs.ctf.tasks.TasksEngine;
import ij.gui.ImageWindow;
import ij.gui.Toolbar;
import ij.io.FileInfo;
import ij.process.StackConverter;
import ij3d.Content;
import ij3d.Image3DUniverse;
import java.awt.Component;
import java.io.File;
import java.util.ArrayList;
import xmipp.Filename;
import xmipp.ImageDouble;

/**
 *
 * @author Juanjo Vega
 */
public class ImagesWindowFactory {

    private final static int UNIVERSE_W = 400, UNIVERSE_H = 400;
//    private final static String TEMPDIR_PATH = System.getProperty("java.io.tmpdir");

    public static void openFilesAsDefault(String files[], boolean poll) {
        for (int i = 0; i < files.length; i++) {
            openFileAsDefault(files[i], poll);
        }
    }

    public static void openFileAsDefault(String filename) {
        openFileAsDefault(filename, false);
    }

    public static void openFileAsDefault(String filename, boolean poll) {
        if (Filename.isSingleImage(filename)) {
            openFileAsImage(filename, poll);
        } else if (Filename.isStackOrVolume(filename)) {
            openFileAsTable(filename);
        } else if (Filename.isMetadata(filename)) {
            openFileAsTable(filename);
        } else {
            openFileAsImage(filename, poll);
        }
    }

    public static void openFilesAsImages(String filenames[], boolean poll) {
        for (int i = 0; i < filenames.length; i++) {
            String filename = Filename.getFilename(filenames[i]);
            long nimage = Filename.getNimage(filenames[i]);

            DEBUG.printMessage(" *** Opening: " + filename + " / nimage: " + nimage);

            openFileAsImage(filename, poll);
        }
    }

    public static void openFileAsImage(String path) {
        openFileAsImage(path, false);
    }

    public static void openFileAsImage(String path, boolean poll) {
        try {
            ImageDouble id = new ImageDouble(path);
            ImagePlus imp = ImageConverter.convertToImagej(id, path);

            openXmippImageWindow(imp, poll);
        } catch (Exception e) {
            IJ.error(e.getMessage() + ": " + path);
        }
    }

    private static ImageWindow openXmippImageWindow(ImagePlus imp, boolean poll) {
        ImageWindow iw = null;

        if (imp != null) {
            if (imp.getStackSize() > 1) {
                iw = new StackWindowOperations(imp, poll);
            } else {
                iw = new ImageWindowOperations(imp, poll);
            }
        }

        return iw;
    }

    public static void openFileAsTable(String filename) {
        File f = new File(filename);

        if (f.exists()) {
            JFrameImagesTable table = new JFrameImagesTable(filename);
            table.setVisible(true);
        } else {
            IJ.error("File not found: " + filename);
        }
    }

    public static void openFilesAsTable(String filenames[]) {
        openFilesAsTable(filenames, false);
    }

    public static void openFilesAsTable(String filenames[], boolean useSameTable) {
        if (useSameTable) {
            JFrameImagesTable table = new JFrameImagesTable(filenames);
            table.setVisible(true);
        } else {
            for (int i = 0; i < filenames.length; i++) {
                openFileAsTable(filenames[i]);
            }
        }
    }

    // Used by micrographs table, to load items marked as selected/unselected.
    public static void openTable(String filenames[], boolean enabled[]) {
        JFrameImagesTable table = new JFrameImagesTable(filenames, enabled);
        table.setVisible(true);
    }

    public static void captureFrame(ImagePlus ip) {
        openXmippImageWindow(ip, false);
    }

    public static void openTableAs3D(AbstractXmippTableModel tableModel) {
        try {
            ArrayList<AbstractTableImageItem> items = tableModel.getAllItems();
            ImagePlus ip = ImageConverter.convertToImagePlus(items);
            ip.setTitle(tableModel.getFilename());

            openImagePlusAs3D(ip);
        } catch (Exception ex) {
            IJ.error(ex.getMessage());
            ex.printStackTrace();
        }
    }

    public static void openTableAsImagePlus(AbstractXmippTableModel tableModel) {
        try {
            String path = tableModel.getFilename();

            // If there is an associated filename, uses it...
            File file = new File(path);
            if (file.exists()) {
//                System.err.println(" +++ EXISTS");
                openFileAsImage(path);
            } else {
//                System.err.println(" !!! EXISTS");
                // ...otherwise, stores it in a temporary file.
                File tempFile = File.createTempFile("tableToStack_", ".stk");
                tempFile.deleteOnExit();

                ArrayList<AbstractTableImageItem> items = tableModel.getAllItems();
                ImagePlus imp = ImageConverter.convertToImagePlus(items);
                IJ.run(imp, "Xmipp writer", "save=" + tempFile.getAbsolutePath());

//                System.err.println(" >>> TMP Saved at: " + file.getAbsolutePath());

                imp.setTitle(tempFile.getName());

                captureFrame(imp);
            }
        } catch (Exception ex) {
            IJ.error(ex.getMessage());
            ex.printStackTrace();
        }
    }

    public static void openImagePlusAsTable(ImagePlus imp) {
        try {
            FileInfo fi = imp.getOriginalFileInfo();
//            System.out.println(" +++ FileInfo: " + fi);

            // If path exists, uses it...
            File file = null;
            if (fi != null && !fi.fileName.trim().isEmpty() && !fi.directory.trim().isEmpty()) {
                file = new File(fi.directory + File.separator + fi.fileName);
            }

            if (file == null || !file.exists()) {   // ...otherwise, stores it in a temporary file.
//                System.err.println(" !!! EXISTS");
                file = File.createTempFile("stackToTable_", ".stk");
                file.deleteOnExit();
                IJ.run(imp, "Xmipp writer", "save=" + file.getAbsolutePath());

//                System.err.println(" >>> TMP Saved at: " + file.getAbsolutePath());
//            } else {
//                System.err.println(" +++ EXISTS");
            }

            openFileAsTable(file.getAbsolutePath());
        } catch (Exception ex) {
            IJ.error(ex.getMessage());
            ex.printStackTrace();
        }
    }

    public static void openImagePlusAs3D(ImagePlus ip) {
        Image3DUniverse universe = new Image3DUniverse(UNIVERSE_W, UNIVERSE_H);

        // Adds the sphere image plus to universe.
        new StackConverter(ip).convertToRGB();
        Content c = universe.addVoltex(ip);
        c.displayAs(Content.VOLUME);

        universe.show();    // Shows...
    }

    public static ImageWindow openCTFImage(ImagePlus ip, String CTFfilename,
            String PSDfilename, TasksEngine tasksEngine,
            String MicrographFilename, int row) {
        IJ.setTool(Toolbar.FREEROI);

        return new CTFRecalculateImageWindow(ip, CTFfilename, PSDfilename,
                tasksEngine, row);
    }

    public static void openMicrograph(String filename) {
        File f = new File(filename);

        if (f.exists()) {
            JFrameMicrographs frame = new JFrameMicrographs(filename);
            frame.setVisible(true);
        } else {
            IJ.error("File is missing", filename + " not found.");
        }
    }

    public static void openFileAsText(String filename, Component parent) {
        JFrameTextFile frameCTF = new JFrameTextFile(filename);
        frameCTF.setLocationRelativeTo(parent);
        frameCTF.setVisible(true);
    }

    public static void openCTFView(ImagePlus imp, String CTFFilename, String PSDFilename) {
        CTFViewImageWindow ctfView = new CTFViewImageWindow(imp, CTFFilename, PSDFilename);
        ctfView.setVisible(true);
    }

    public static String getTitle(String title, int width) {
        int strlenght = title.length() / 4; // Approximated string graphical lenght.

        int toremove = strlenght - width;

        int start = toremove > 0 ? toremove : 0;

        String newtitle = title.substring(start, title.length());
        String prefix = toremove > 0 ? "..." : "";

        return prefix + newtitle;
    }
}
