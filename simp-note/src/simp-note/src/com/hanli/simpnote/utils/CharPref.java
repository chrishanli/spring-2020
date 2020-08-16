package com.hanli.simpnote.utils;

import javafx.scene.paint.Color;
import javafx.scene.text.Font;

// 选择一个字体
public final class CharPref {
    private Font font;
    private Color color;

    public CharPref() {
        this.setFont(Font.font(13));
        this.setColor(Color.BLACK);
    }

    public Font getFont() {
        return font;
    }

    public void setFont(Font font) {
        this.font = font;
    }

    public Color getColor() {
        return color;
    }

    public void setColor(Color color) {
        this.color = color;
    }
}