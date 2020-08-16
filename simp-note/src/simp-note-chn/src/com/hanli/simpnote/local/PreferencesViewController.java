package com.hanli.simpnote.local;

import com.hanli.simpnote.utils.CharPref;
import com.hanli.simpnote.utils.NotePadUtils;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.stage.Stage;

public class PreferencesViewController {

    @FXML
    private Label lblFontName;

    @FXML
    private ComboBox<NotePadUtils.Encoding> combOpenEncode;

    @FXML
    private ComboBox<NotePadUtils.Encoding> combSaveEncode;

    @FXML
    private CheckBox chkAttachTxt;

    @FXML
    private Button btnSave;

    private CharPref selectedTextFont;

    private SysPreference sysPreference;

    private boolean isSaved;

    @FXML private void initialize() {
        if (selectedTextFont == null) {
            this.selectedTextFont = new CharPref();
        }
        updateFontName(selectedTextFont);
        updateEncodingName(combOpenEncode);
        updateEncodingName(combSaveEncode);
    }

    @FXML
    void btnAttachTxtClicked() {

    }

    @FXML
    void btnCancelClicked() {
        Stage w = (Stage) this.btnSave.getScene().getWindow();
        w.close();
    }

    @FXML
    void btnSaveClicked() {
        this.isSaved = true;
        // 更新fp中的内容
        sysPreference.setCurrFont(this.selectedTextFont);
        sysPreference.setCurrOpenEncoding(this.combOpenEncode.getValue());
        sysPreference.setCurrSaveEncoding(this.combSaveEncode.getValue());
        sysPreference.setTxtNeeded(this.chkAttachTxt.isSelected());

        Stage w = (Stage) this.btnSave.getScene().getWindow();
        w.close();
    }

    private void updateFontName(CharPref charPref) {
        // 设置字形
        this.lblFontName.setFont(selectedTextFont.getFont());
        // 设置字体名
        this.lblFontName.setText(String.format("%s %.0fpt",
                charPref.getFont().getName(),
                charPref.getFont().getSize()));
    }

    private void updateEncodingName(ComboBox<NotePadUtils.Encoding> cb) {
        ObservableList<NotePadUtils.Encoding> options =
                FXCollections.observableArrayList(NotePadUtils.Encoding.values());
        cb.setItems(options);
    }

    @FXML
    void btnSelFontClicked() {
        // 显示字体、颜色窗口
        CharPref charPref = NotePadUtils.changeCharPref(true);
        if (charPref != null) {
            // 非空，则设置该字体
            this.lblFontName.setFont(charPref.getFont());
            // 保存及显示该字体
            this.selectedTextFont = charPref;
            updateFontName(selectedTextFont);
        }
    }

    public void setSysPreference(SysPreference sysPreference) {
        this.sysPreference = sysPreference;

        this.selectedTextFont = sysPreference.getCurrTextStyle();
        updateFontName(selectedTextFont);
        combOpenEncode.getSelectionModel().select(sysPreference.getCurrOpenEncoding());
        combSaveEncode.getSelectionModel().select(sysPreference.getCurrSaveEncoding());
        chkAttachTxt.setSelected(sysPreference.isTxtNeeded());
    }

    public boolean isSaved() {
        return isSaved;
    }
}
