package com.hanli.simpnote.about;

import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;

import java.util.Properties;

public class AboutViewController {

    @FXML
    private ImageView imgSimpNote;

    @FXML
    private Label lblBuild;

    @FXML
    private ImageView imgHanLi;

    @FXML
    private Label lblRuntimeVer;

    @FXML
    private Label lblVMVer;

    public void initialize() {
        Image iconSimpNote = new Image("com/hanli/simpnote/resources/simp_note_white.png");
        imgSimpNote.setImage(iconSimpNote);
        Image iconHanLi = new Image("com/hanli/simpnote/resources/logo_small.png");
        imgHanLi.setImage(iconHanLi);

        // 获取运行时
        Properties pp = System.getProperties();
        lblRuntimeVer.setText(pp.getProperty("java.specification.version"));
        lblVMVer.setText(pp.getProperty("java.vm.version"));
        lblBuild.setText("1029");
    }

}
