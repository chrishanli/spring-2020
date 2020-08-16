package com.hanli.simpnote.notepad;

import com.hanli.simpnote.utils.CharPref;
import javafx.geometry.Bounds;
import javafx.geometry.Point2D;
import javafx.scene.input.InputMethodRequests;
import org.fxmisc.richtext.StyleClassedTextArea;

import java.util.Optional;

public class Notepad extends StyleClassedTextArea {

    public Notepad() {
        super();
        // 支援中文
        this.setInputMethodRequests(new InputMethodRequests() {
            @Override
            public Point2D getTextLocation(int offset) {
                Optional<Bounds> caretPositionBounds = getCaretBounds();
                if (caretPositionBounds.isPresent()) {
                    Bounds bounds = caretPositionBounds.get();
                    return new Point2D(bounds.getMaxX(), bounds.getMaxY());
                }
                return new Point2D(0, 0);
            }
            @Override
            public int getLocationOffset(int x, int y) {
                return 0;
            }
            @Override
            public void cancelLatestCommittedText() { }
            @Override
            public String getSelectedText() {
                return "";
            }
        });
    }

    // 设置字体及字形大小
    public void setStyle(CharPref charPref) {
        String style = "-fx-font-family:" + charPref.getFont().getFamily() +
                "; -fx-font-size:" + charPref.getFont().getSize();
        super.setStyle(style);
    }

}
