package com.hanli.simpnote;

import com.hanli.simpnote.local.RecentDocument;
import com.hanli.simpnote.notepad.NotePadWindow;
import com.hanli.simpnote.utils.NotePadUtils;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.stage.Stage;

import java.io.File;
import java.util.ArrayList;

public class InitialViewController {

    @FXML
    private Label lblVersion;

    @FXML
    private ListView<RecentDocument> listRecent;

    @FXML
    private Button btnNewDoc;

    @FXML
    private Button btnOpen;

    @FXML
    private ImageView imgSN;

    @FXML
    void initialize() {
        // 设置logo
        Image iconSimpNote = new Image(getClass().getResource("resources/simp_note.png").toString());
        this.imgSN.setImage(iconSimpNote);
        this.lblVersion.setText(NotePadUtils.version);

        // 设置最近访问文件
        ArrayList<RecentDocument> docsArrayList = NotePadUtils.getRecentDocs();
        if (docsArrayList == null) {
            docsArrayList = new ArrayList<>();
            NotePadUtils.setRecentDocs(docsArrayList);
        }
        ObservableList<RecentDocument> docs = FXCollections.observableArrayList(docsArrayList);
        listRecent.setItems(docs);
        listRecent.setCellFactory((ListView<RecentDocument> l) -> new RecentDocCell());
        listRecent.getSelectionModel()
                .selectedIndexProperty()
                .addListener((observable, old_val, new_val) -> {
                    String filePath = listRecent
                            .getItems()
                            .get(new_val.intValue())
                            .getFileAbsolutePath();
                    // 打开选中的文件
                    File file = new File(filePath);
                    if (file.exists()) {
                        NotePadWindow notePadWindow = new NotePadWindow(file);
                        notePadWindow.show();
                        // 关闭窗体
                        Stage stage = (Stage)btnNewDoc.getScene().getWindow();
                        stage.close();
                    } else {
                        NotePadUtils.reportError("文件访问失败",
                                "该文件不存在，或是SimpNote并无权限访问它。");
                    }
                });
    }

    static class RecentDocCell extends ListCell<RecentDocument> {
        @Override
        protected void updateItem(RecentDocument item, boolean empty) {
            super.updateItem(item, empty);
            if (!empty) {
                // 生成一个包含文件信息的VBox
                double maxWidth = getListView().getWidth() - 8;
                Label lblFileName = new Label(item.getFileName());
                Label lblFilePath = new Label(item.getFileAbsolutePath());
                lblFileName.setMaxWidth(maxWidth);
                lblFilePath.setMaxWidth(maxWidth);
                lblFilePath.setTextFill(Color.GRAY);
                VBox vb = new VBox();
                vb.getChildren().addAll(lblFileName, lblFilePath);
                // 将VBox装入这个Cell
                this.setGraphic(vb);
            }
        }
    }

    @FXML
    void btnNewDocClicked() {
        // 新建newDoc窗口
        NotePadUtils.makeNewDocument("");
        // 关闭窗体
        Stage stage = (Stage)btnNewDoc.getScene().getWindow();
        stage.close();
    }


    @FXML
    void btnNewFromClipClicked() {
        // 获取剪贴板内容
        String clpBrdMatter = NotePadUtils.getSysClipboardText();
        // 新建newDoc窗口
        NotePadUtils.makeNewDocument(clpBrdMatter);
        // 关闭窗体
        Stage stage = (Stage)btnNewDoc.getScene().getWindow();
        stage.close();
    }

    @FXML
    void btnOpenClicked() {
        Stage stage = (Stage)btnOpen.getScene().getWindow(); // 获取当前窗口
        // 用打开文件新建newDoc窗口
        File opened = NotePadUtils.makeOpenDocument(stage);
        if (opened != null) {
            // 成功打开文件，则关闭窗体
            stage.close();
            // 记载进数据库
            NotePadUtils.addRecentDoc(opened);
        }
    }

}
