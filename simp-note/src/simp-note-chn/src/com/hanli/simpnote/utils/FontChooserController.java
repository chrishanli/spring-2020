package com.hanli.simpnote.utils;

import java.awt.GraphicsEnvironment;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.ResourceBundle;

import javafx.collections.FXCollections;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.ListView;
import javafx.scene.layout.HBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.FontPosture;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;

// 字体选择器
public class FontChooserController implements Initializable {
    @FXML
    private Label font;
    @FXML
    private Label type;
    @FXML
    private Label size;
    @FXML
    private ListView<Text> fontList;
    @FXML
    private ListView<Text> typeList;
    @FXML
    private ListView<String> sizeList;
    @FXML
    private ComboBox<HBox> colorList;
    @FXML
    private Button ok;
    @FXML
    private Button cancel;
    @FXML
    private Label sample;
    // 颜色映射列表
    private Map<String, Color> colorMap;
    // 字号映射列表
    private Map<String, Integer> sizeMap;
    // 默认字形(非加粗)
    private FontWeight weiStyle = FontWeight.NORMAL;
    // 默认字形(非斜体)
    private FontPosture posStyle = FontPosture.REGULAR;
    // 获取到的字体对象
    private CharPref selectedFont = null;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        // 获取系统字体列表
        GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
        String[] fontNames = ge.getAvailableFontFamilyNames();

        List<Text> textList = new ArrayList<>(fontNames.length);
        for (int i = 0; i < fontNames.length; i++) {
            // 将fontNames.length个Text对象添加到textList中
            textList.add(new Text(fontNames[i]));
            // 设置字体列表字体
            textList.get(i).setFont(Font.font(fontNames[i]));
        }
        // 将获得的字体列表放入字体ListView中
        fontList.setItems(FXCollections.observableArrayList(textList));
        // 设置默认选中的字体
        fontList.getSelectionModel().selectFirst();
        // 显示选中的字体名称
        font.setText(fontList.getSelectionModel().getSelectedItem().getText());

        // 字体ListView的监听事件
        fontList.getSelectionModel()
                .selectedItemProperty()
                .addListener(event -> { // 选中字体
                    // 显示选中的字体名称
                    font.setText(fontList.getSelectionModel().getSelectedItem().getText());
                    // 更新新字体预览效果
                    sample.setFont(Font.font(fontList.getSelectionModel().getSelectedItem().getText(), weiStyle,
                            posStyle,
                            sizeMap.get(sizeList.getSelectionModel().getSelectedItem())));
                });
        // 实例化字形列表内容
        Text normal_regular = new Text("Normal");
        Text bold_regular = new Text("Bold");
        Text normal_italic = new Text("Italic");
        Text bold_italic = new Text("Bold Italic");
        // 设置字形列表字体
        normal_regular.setFont(Font.font(Font.getDefault().getName(), FontWeight.NORMAL, FontPosture.REGULAR, Font
                .getDefault().getSize()));
        bold_regular.setFont(Font.font(Font.getDefault().getName(), FontWeight.BOLD, FontPosture.REGULAR, Font
                .getDefault().getSize()));
        normal_italic.setFont(Font.font(Font.getDefault().getName(), FontWeight.NORMAL, FontPosture.ITALIC, Font
                .getDefault().getSize()));
        bold_italic.setFont(Font.font(Font.getDefault().getName(), FontWeight.BOLD, FontPosture.ITALIC, Font
                .getDefault().getSize()));
        // 将字体类型名称放入字形ListView中
        typeList.setItems(FXCollections.observableArrayList(normal_regular, bold_regular, normal_italic, bold_italic));
        // 设置默认选中的字称
        typeList.getSelectionModel().selectFirst();
        // 显示选中的字形名称
        type.setText(typeList.getSelectionModel().getSelectedItem().getText());
        // 字形ListView监听事件
        typeList.getSelectionModel()
                .selectedItemProperty()
                .addListener(event -> { // 选中字形
                    String value = typeList.getSelectionModel().getSelectedItem().getText();
                    switch (value) {
                        case "Normal":
                            weiStyle = FontWeight.NORMAL;
                            posStyle = FontPosture.REGULAR;
                            break;
                        case "Bold":
                            weiStyle = FontWeight.BOLD;
                            posStyle = FontPosture.REGULAR;
                            break;
                        case "Italic":
                            weiStyle = FontWeight.NORMAL;
                            posStyle = FontPosture.ITALIC;
                            break;
                        case "Bold Italic":
                            weiStyle = FontWeight.BOLD;
                            posStyle = FontPosture.ITALIC;
                            break;
                    }
                    type.setText(value);
                    // 更新预览文字的字形效果
                    sample.setFont(Font.font(fontList.getSelectionModel().getSelectedItem().getText(), weiStyle,
                            posStyle,
                            sizeMap.get(sizeList.getSelectionModel().getSelectedItem())));
                });
        // 字号字符串
        String[] sizeStr = { "8", "9", "10", "11", "12", "14", "16", "18", "20", "22",
                "24", "26", "28", "36", "48", "72", "初号", "小初", "一号", "小一", "二号", "小二", "三号", "小三", "四号", "小四", "五号",
                "小五", "六号", "小六", "七号", "八号" };
        // 字号值
        int[] sizeValue = { 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72, 42, 36, 26, 24, 22, 18, 16,
                15, 14, 12, 11, 9, 8, 7, 6, 5 };
        sizeList.setItems(FXCollections.observableArrayList(sizeStr));

        // 实例化字号映射表
        sizeMap = new HashMap<>();
        // 循环得到每个字号字符串对应的值
        for (int i = 0; i < sizeList.getItems().size(); ++i) {
            sizeMap.put(sizeStr[i], sizeValue[i]);
        }
        // 设置默认选中字号（18 pt）
        sizeList.getSelectionModel().select(7);
        // 显示选中的字号
        size.setText(sizeList.getSelectionModel().getSelectedItem());
        // 字号ListView监听事件
        sizeList.getSelectionModel().selectedItemProperty().addListener(event -> {
            size.setText(sizeList.getSelectionModel().getSelectedItem());
            sample.setFont(Font.font(fontList.getSelectionModel().getSelectedItem().getText(), weiStyle, posStyle,
                    sizeMap.get(sizeList.getSelectionModel().getSelectedItem())));
        });

        // 颜色字符串列表
        String[] colorStr = { "Black", "Blue", "Cyan", "Dark Gray", "Gray", "Green", "Light Gray", "Magenta", "Orange",
                "Pink", "Red", "White", "Yellow" };

        // 颜色值列表
        Color[] colorValue = new Color[] {
                Color.BLACK, Color.BLUE, Color.CYAN, Color.DARKGRAY, Color.GRAY, Color.GREEN, Color.LIGHTGRAY,
                Color.MAGENTA, Color.ORANGE, Color.PINK, Color.RED, Color.WHITE, Color.YELLOW
        };

        List<HBox> colourPrevList = new ArrayList<>(colorValue.length);
        for (int i = 0; i < colorValue.length; i++) {
            // 颜色预览长方形
            Rectangle rect = new Rectangle(70, 15);
            rect.setFill(colorValue[i]);
            HBox hBox = new HBox();
            Label colourName = new Label(colorStr[i]);
            colourName.setTextFill(colorValue[0]);
            hBox.setSpacing(5);
            hBox.getChildren().addAll(colourName, rect);
            colourPrevList.add(hBox);
        }

        // 将颜色字符串放入颜色ChoiceBox中
        colorList.setItems(FXCollections.observableArrayList(colourPrevList));
        // 设置默认颜色
        colorList.getSelectionModel().select(0);

        // 实例化颜色映射列表
        colorMap = new HashMap<>();
        // 循环得到每个颜色字符串对应的颜色值
        for (int i = 0; i < colorList.getItems().size(); i++) {
            colorMap.put(colorStr[i], colorValue[i]);
        }

        // 颜色ComboBox监听事件
        colorList.getSelectionModel().selectedItemProperty().addListener(event ->
                colorList.getSelectionModel()
                        .getSelectedItem()
                        .getChildren()
                        .forEach(node -> {
            if (node.getClass().equals(Label.class)) {
                Label label = (Label) node;
                // 更新预览文字颜色
                sample.setTextFill(colorMap.get(label.getText()));
            }
        }));

        // 示例文字初始化样式
        sample.setFont(Font.font(fontList.getSelectionModel()
                        .getSelectedItem()
                        .getText(),
                weiStyle, posStyle,
                sizeMap.get(sizeList.getSelectionModel().getSelectedItem())));

        // 确定按钮点击事件
        ok.setOnAction(event -> {
            // 将获取到的字体信息保存到字体对象中
            selectedFont = new CharPref();
            // 保存字体、字形及字号
            selectedFont.setFont(Font.font(fontList.getSelectionModel().getSelectedItem().getText(), weiStyle,
                    posStyle,
                    sizeMap.get(sizeList.getSelectionModel().getSelectedItem())));

            colorList.getSelectionModel().getSelectedItem().getChildren().forEach(node -> {
                if (node.getClass().equals(Rectangle.class)) {
                    Rectangle rect = (Rectangle) node;
                    // 保存颜色
                    selectedFont.setColor((Color) rect.getFill());
                }
            });

            // 关闭窗口
            cancel.getScene().getWindow().hide();
        });

        // 取消按钮点击事件
        cancel.setOnAction(event -> {
            // 关闭窗口
            cancel.getScene().getWindow().hide();
        });

    }

    public CharPref getSelectedFont() { return selectedFont; }

    public void setColourListDisable(boolean isDisable) {
        this.colorList.setDisable(isDisable);
    }

}