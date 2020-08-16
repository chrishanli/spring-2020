package com.hanli.simpnote.statistics;

import com.hanli.simpnote.notepad.Notepad;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

public class StatisticsWindow {

    // 需要分析的文字
    private final String targetArticle;

    public StatisticsWindow(Notepad tw) {
        this.targetArticle = tw.getText();
    }

    public StatisticsWindow(String ta) {
        this.targetArticle = ta;
    }

    public void show() throws IOException {
        // 窗体配置
        FXMLLoader loader = new FXMLLoader(getClass().getResource("StatisticsView.fxml"));
        Parent rootView = loader.load();
        Scene sc = new Scene(rootView);
        Stage window = new Stage();
        window.setScene(sc);
        window.setTitle("文本统计");
        // 获取Controller
        StatisticsViewController nc = loader.getController();
        nc.setTargetArticle(this.targetArticle);
        // 显示窗体
        window.show();
    }
}
