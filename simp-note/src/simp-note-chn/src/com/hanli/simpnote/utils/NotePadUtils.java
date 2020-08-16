package com.hanli.simpnote.utils;

import com.hanli.simpnote.local.RecentDocument;
import com.hanli.simpnote.local.RecentDocuments;
import com.hanli.simpnote.local.SysPreference;
import com.hanli.simpnote.notepad.NotePadWindow;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonBar;
import javafx.scene.control.ButtonType;
import javafx.scene.input.Clipboard;
import javafx.scene.input.ClipboardContent;
import javafx.scene.input.DataFormat;
import javafx.stage.FileChooser;
import javafx.stage.Modality;
import javafx.stage.Stage;

import javax.xml.bind.*;
import java.io.*;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Locale;
import java.util.Optional;

public class NotePadUtils {

    // 编码方式表
    public enum Encoding {
        UTF8("UTF-8", "utf-8"),
        GBK("GBK", "gbk"),
        BIG5("Big-5", "big5");
        String exp, sys;
        Encoding(String exp, String sys) { this.exp = exp; this.sys = sys;}
        @Override
        public String toString() { return exp; }
        public String getSysName() { return this.sys; }
    }
    // 版本号
    public static final String version = "2020.1029";

    // 系统设置
    private static SysPreference sp;
    // 最近读取的文件
    private static RecentDocuments recentDocuments = null;

    // 系统剪贴板
    private static final Clipboard clipboard = javafx.scene.input.Clipboard.getSystemClipboard();
    // 系统剪贴板之内容
    private static final ClipboardContent content = new ClipboardContent();

    public static String getSysClipboardText() {
        String ret = "";
        // 获取剪切板中的内容
        Object clipTf = clipboard.getContent(DataFormat.PLAIN_TEXT);
        if (clipTf != null) {
            ret = (String) clipTf;
        }
        return ret;
    }

    public static void appendToSysClipboardText(String toAppend) {
        content.putString(toAppend);
        clipboard.setContent(content);
    }

    // 判断是否为macOS
    public static boolean isMacOS() {
        String OS = System.getProperty("os.name", "generic").toLowerCase(Locale.ENGLISH);
        return (OS.contains("mac")) || (OS.contains("darwin"));
    }

    public static void makeNewDocument(String initString) {
        // 新建newDoc窗口
        NotePadWindow notePadWindow = new NotePadWindow(initString);
        notePadWindow.show();
    }

    public static File makeOpenDocument(Stage stage) {
        // 打开dialogue
        FileChooser chooser = new FileChooser(); // 创建一个文件对话框
        chooser.setTitle("Open File..."); // 设置文件对话框的标题
        chooser.setInitialDirectory(new File(".")); // 设置文件对话框的初始目录
        // 文件类型过滤器
        chooser.getExtensionFilters().addAll(
                new FileChooser.ExtensionFilter("All Files", "*.*"),
                new FileChooser.ExtensionFilter("Plain Text", "*.txt", "*.md", "*.xml", "*.c"));
        // 显示文件打开对话框
        File file = chooser.showOpenDialog(stage); // 显示文件打开对话框
        if (file != null) { // 文件对象为空，表示没有选择任何文件
            // 打开newDoc窗口
            NotePadWindow notePadWindow = new NotePadWindow(file);
            notePadWindow.show();
            return file;
        } else {
            return null;
        }
    }

    // 保存文本
    public static boolean saveText(String data, File outfile, String encoding) {
        FileOutputStream out;
        try {
            out = new FileOutputStream(outfile);
        } catch (FileNotFoundException e) {
            // 文件未找到，则新建一个
            try {
                boolean isOK = outfile.createNewFile();
                if (!isOK) {
                    NotePadUtils.reportError("File Exists", "Error occurred while creating new file: file exists.");
                    return false;
                }
                out = new FileOutputStream(outfile);
            } catch (IOException newFileEx) {
                // 创建文件错误
                NotePadUtils.reportError("File was unable to create", "Error occurred while creating new file: permission denied.");
                return false;
            }
        }

        // 尝试写入文件
        try {
            out.write(data.getBytes(encoding));
        } catch (IOException e) {
            NotePadUtils.reportError("Unable to write file", "Error occurred while writing file: IO error.");
            return false;
        }

        return true;
    }

    // 弹框报告错误
    public static void reportError(String title, String desc) {
        Alert al = new Alert(Alert.AlertType.ERROR);
        al.setTitle("Error");
        al.setHeaderText(title);
        al.setContentText(desc);
        al.showAndWait();
    }

    // 弹框报告提示
    public static void reportInfo(String title, String desc) {
        Alert al = new Alert(Alert.AlertType.INFORMATION);
        al.setTitle("Info");
        al.setHeaderText(title);
        al.setContentText(desc);
        al.showAndWait();
    }

    // 弹框询问
    public static boolean askUser(String title, String desc) {
        Alert al = new Alert(Alert.AlertType.CONFIRMATION);
        al.setTitle("Confirm");
        al.setHeaderText(title);
        al.setContentText(desc);
        Optional<ButtonType> sel = al.showAndWait();
        return sel.map(buttonType ->
                (buttonType
                .getButtonData()
                .equals(ButtonBar.ButtonData.OK_DONE)))
                .orElse(false);
    }

    public static CharPref changeCharPref(boolean isColourListDisable) {
        // 窗体配置
        FXMLLoader loader = new FXMLLoader(NotePadUtils.class.getResource("FontChooser.fxml"));
        Parent rootView;
        try {
            rootView = loader.load();
        } catch (IOException e) {
            reportError("Font selector could not be opened at this time.", "Unknown error occurred: " + e.getLocalizedMessage());
            return null;
        }

        Scene sc = new Scene(rootView);
        Stage window = new Stage();
        window.initModality(Modality.APPLICATION_MODAL);
        window.setScene(sc);
        window.setTitle("Select Font");
        // 获取Controller
        FontChooserController nc = loader.getController();
        nc.setColourListDisable(isColourListDisable);
        // 显示窗体
        window.showAndWait();

        return nc.getSelectedFont();
    }

    // 读取当前系统（已经读入）的设置文档
    public static SysPreference getSysPreference() {
        return sp;
    }

    // 读入设置文档
    public static void readSysPreference(String prefLocation) {
        // 创建JAXB context
        try {
            JAXBContext context = JAXBContext.newInstance(SysPreference.class);
            Unmarshaller unmarshaller = context.createUnmarshaller();
            try (BufferedReader in = Files.newBufferedReader(Paths.get(prefLocation).toAbsolutePath())) {
                sp = (SysPreference) unmarshaller.unmarshal(in);
            } catch (IOException e) {
                if (e instanceof NoSuchFileException) {
                    // 新建一个pref文件
                    try {
                        Files.createFile(Paths.get(prefLocation).toAbsolutePath());
                        System.out.println("Create system pref file succeeded.");
                        BufferedWriter bw = Files.newBufferedWriter(Paths.get(prefLocation).toAbsolutePath());
                        bw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<pref/>");
                        bw.close();
                    } catch (IOException ex) {
                        NotePadUtils.reportError("SimpNote could not open at this time.", "System Preferences not found, and could not create a new one as well: "+ ex);
                        System.exit(1);
                    }
                } else {
                    NotePadUtils.reportError("SimpNote could not open at this time.", "Unknown error: "+e);
                    System.exit(1);
                }
            }
        } catch (Exception e) {
            NotePadUtils.reportError("SimpNote could not open at this time.", "Marshaller error: "+e);
            System.exit(1);
        }
    }

    // 写设置文档
    public static void writeSysPreference(String prefLocation) {
        // 创建JAXB context
        if (sp != null) {
            try {
                JAXBContext context = JAXBContext.newInstance(SysPreference.class);
                Marshaller marshaller = context.createMarshaller();
                marshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, Boolean.TRUE);
                try (BufferedWriter out = Files.newBufferedWriter(Paths.get(prefLocation).toAbsolutePath())) {
                    marshaller.marshal(sp, out);
                } catch (IOException e) {
                    NotePadUtils.reportError("SimpNote exit with an error.", "Your preferences could not be saved, because: "+e);
                }
            } catch (Exception e) {
                NotePadUtils.reportError("SimpNote exit with an error.", "Your preferences could not be saved, because: "+e);
            }
        }
    }

    // 读入最近读取之文件文档
    public static void readRecentDocs(String rdLocation) {
        // 设置最近访问之文件
        RecentDocuments rd = null;
        Path p = Paths.get(rdLocation).toAbsolutePath();
        try {
            JAXBContext context = JAXBContext.newInstance(RecentDocuments.class);
            Unmarshaller unmarshaller = context.createUnmarshaller();
            BufferedReader br = Files.newBufferedReader(p);
            rd = (RecentDocuments) unmarshaller.unmarshal(br);
        } catch (IOException | JAXBException e) {
            if (e instanceof NoSuchFileException) {
                // 创建新文件
                try {
                    Files.createFile(p.toAbsolutePath());
                    System.out.println("Create recent file succeeded.");
                    BufferedWriter bw = Files.newBufferedWriter(p);
                    bw.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<recentDocuments />");
                    bw.close();
                    rd = new RecentDocuments();
                    rd.setDocs(new ArrayList<>());
                } catch (IOException ex) {
                    NotePadUtils.reportError("SimpNote could not open at this time.", "Recent-documents file not found, and could not create a new one as well: "+ ex);
                    System.exit(1);
                }
            } else {
                e.printStackTrace();
                NotePadUtils.reportError("SimpNote could not open at this time.", "Unknown error: "+ e);
                System.exit(1);
            }
        }
        recentDocuments = rd;
    }

    // 读取已经读入的最近读取之文件
    public static ArrayList<RecentDocument> getRecentDocs() {
        return recentDocuments.getDocs();
    }

    // 设置已经读入的最近读取之文件
    public static void setRecentDocs(ArrayList<RecentDocument> rd) {
        recentDocuments.setDocs(rd);
    }

    //
    public static void addRecentDoc(File file) {
        RecentDocument rd = new RecentDocument();
        rd.setFileAbsolutePath(file.getAbsolutePath());
        rd.setFileName(file.getName());
        recentDocuments.getDocs().add(rd);
    }

    // 写最近读取之文件文档
    public static void writeRecentDocs(String rdLocation) {
        // 创建JAXB context
        if (recentDocuments != null) {
            try {
                Marshaller marshaller = JAXBContext.newInstance(RecentDocuments.class).createMarshaller();
                marshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, Boolean.TRUE);
                try (BufferedWriter out = Files.newBufferedWriter(Paths.get(rdLocation).toAbsolutePath())) {
                    marshaller.marshal(recentDocuments, out);
                } catch (IOException e) {
                    e.printStackTrace();
                    NotePadUtils.reportError("SimpNote exit with an error.", "Recent file could not be saved: "+e);
                }
            } catch (Exception e) {
                e.printStackTrace();
                NotePadUtils.reportError("SimpNote exit with an error.", "Recent file could not be saved, because: "+e);
            }
        }
    }
}
