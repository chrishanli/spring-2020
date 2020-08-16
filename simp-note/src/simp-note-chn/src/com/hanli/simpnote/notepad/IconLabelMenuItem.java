package com.hanli.simpnote.notepad;

import de.jensd.fx.glyphs.fontawesome.FontAwesomeIcon;
import de.jensd.fx.glyphs.fontawesome.FontAwesomeIconView;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.MenuItem;
import javafx.scene.input.KeyCodeCombination;

// 这是一个含有图标及快捷键的菜单项
public class IconLabelMenuItem extends MenuItem {

    public IconLabelMenuItem(FontAwesomeIcon icon, String itemTitle, KeyCodeCombination accelerator, EventHandler<ActionEvent> eventHandler) {
        this(icon, itemTitle, eventHandler);
        this.setAccelerator(accelerator);
    }

    public IconLabelMenuItem(FontAwesomeIcon icon, String itemTitle, EventHandler<ActionEvent> eventHandler) {
        super();

        FontAwesomeIconView iconView;
        if (icon == null) {
            iconView = new FontAwesomeIconView();
            iconView.setVisible(false);
        } else {
            iconView = new FontAwesomeIconView(icon);
        }

        super.setGraphic(iconView);
        super.setText(itemTitle);
        super.setStyle("-fx-font-family: Arial; -fx-font-size: 15");
        super.setOnAction(eventHandler);
    }
}