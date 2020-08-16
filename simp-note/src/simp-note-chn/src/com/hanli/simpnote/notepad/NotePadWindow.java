package com.hanli.simpnote.notepad;

import com.hanli.simpnote.utils.NotePadUtils;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Modality;
import javafx.stage.Stage;

import java.io.File;
import java.io.IOException;

public class NotePadWindow {
    // 文档的file
    private File file = null;
    // 初识字符串（如有）
    private String initialString;

    // 打开文件
    public NotePadWindow(File file) {
        this.file = file;
    }

    // 新建文件
    public NotePadWindow(String initialString) {
        this.initialString = initialString;
    }

    public void show() {
        // 窗体配置
        FXMLLoader loader = new FXMLLoader(getClass().getResource("NotePadView.fxml"));
        Parent rootView;
        try {
            rootView = loader.load();
        } catch (IOException e) {
            e.printStackTrace();
            NotePadUtils.reportError("SimpNote现时不可用", "Unknown error: " + e.getLocalizedMessage());
            return;
        }
        Scene sc = new Scene(rootView);
        Stage window = new Stage();
        window.initModality(Modality.NONE);
        window.setScene(sc);
        // 获取Controller
        NotePadViewController nc = loader.getController();

        // 判断是打开文件还是从剪贴板新建文件
        if (file != null) {
            window.setTitle(file.getName() + " - SimpNote");
            nc.setDisplay(file);
        } else {
            window.setTitle("未命名 - SimpNote");
            nc.setDisplay(this.initialString);
        }

        // 设置关闭窗口事件
        window.setOnCloseRequest(windowEvent -> {
            nc.btnCloseDocClicked(null);
            windowEvent.consume();
        });

        // 显示窗体
        window.show();
    }
}
