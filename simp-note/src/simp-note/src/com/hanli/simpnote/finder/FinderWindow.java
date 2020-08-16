package com.hanli.simpnote.finder;
import com.hanli.simpnote.notepad.Notepad;
import com.hanli.simpnote.utils.NotePadUtils;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Modality;
import javafx.stage.Stage;

import java.io.IOException;

public class FinderWindow {

    // 待操作的notepad
    private final Notepad notepad;
    private String initString = "";
    EventHandler<ActionEvent> changeSelectionCompletion;

    public FinderWindow(Notepad targetNotepad) {
        this.notepad = targetNotepad;
    }

    public FinderWindow(Notepad targetNotepad, String initString) {
        this.notepad = targetNotepad;
        this.initString = initString;
    }

    // 显示，指定当光标发生变化之后的操作
    public void show(EventHandler<ActionEvent> completion) {
        this.changeSelectionCompletion = completion;
        show();
    }

    // 显示
    public void show() {
        // 窗体配置
        FXMLLoader loader = new FXMLLoader(getClass().getResource("FinderView.fxml"));
        Parent rootView;
        try {
            rootView = loader.load();
        } catch (IOException e) {
            NotePadUtils.reportError("Finder could not be opened at this time.", "Error initiating: " + e.getLocalizedMessage());
            return;
        }
        Scene sc = new Scene(rootView);
        Stage window = new Stage();
        window.initModality(Modality.NONE);
        window.setTitle("Find / Replace");
        window.setResizable(false);
        window.setScene(sc);
        // 获取Controller
        FinderViewController nc = loader.getController();
        nc.setTargetNotePad(this.notepad);
        nc.setInitString(this.initString);
        if (this.changeSelectionCompletion != null) {
            nc.setOnSelectionMayChanged(this.changeSelectionCompletion);
        }

        window.showAndWait();
    }
}
