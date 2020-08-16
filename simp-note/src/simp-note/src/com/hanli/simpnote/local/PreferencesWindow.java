package com.hanli.simpnote.local;

import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Modality;
import javafx.stage.Stage;

import java.io.IOException;

public class PreferencesWindow {

    private SysPreference sysPreference;
    private boolean isSaved;

    public void show() throws IOException {
        // 窗体配置
        FXMLLoader loader = new FXMLLoader(getClass().getResource("PreferencesView.fxml"));
        Parent rootView = loader.load();
        Scene sc = new Scene(rootView);
        Stage window = new Stage();
        window.initModality(Modality.APPLICATION_MODAL);
        window.setScene(sc);
        window.setResizable(false);
        window.setTitle("System Preferences");
        // 获取Controller
        PreferencesViewController nc = loader.getController();
        nc.setSysPreference(this.sysPreference);

        // 显示窗体
        window.showAndWait();
        this.isSaved = nc.isSaved();
    }

    public void setSysPreference(SysPreference fp) {
        this.sysPreference = fp;
    }

    public boolean isSaved() {
        return this.isSaved;
    }
}
