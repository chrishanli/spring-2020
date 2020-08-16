package com.hanli.simpnote;
import com.hanli.simpnote.utils.NotePadUtils;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

public class SimpNoteMain extends Application {

    private static Stage stage;

    public static void showInitPage() {
        FXMLLoader loader = new FXMLLoader(SimpNoteMain.class.getResource("InitialView.fxml"));
        Parent root;
        try {
            root = loader.load();
        } catch (IOException e) {
            e.printStackTrace();
            NotePadUtils.reportError("SimpNote现时不可用", "未知错误: " + e.getLocalizedMessage());
            return;
        }

        stage.setResizable(false);
        stage.setTitle("欢迎 - SimpNote");
        Scene baseScene = new Scene(root);
        stage.setScene(baseScene);
        stage.show();
    }

    @Override
    public void start(Stage primaryStage) {
        // 读入用户设置
        NotePadUtils.readSysPreference("pref/simp-note-pref.xml");
        NotePadUtils.readRecentDocs("pref/simp-note-recent.xml");
        stage = primaryStage;
        showInitPage();
    }

    public static void main(String[] args) {
        launch(args);
        // 写出用户设置
        NotePadUtils.writeSysPreference("pref/simp-note-pref.xml");
        NotePadUtils.writeRecentDocs("pref/simp-note-recent.xml");
        System.out.println("Programme exited.");
    }
}
