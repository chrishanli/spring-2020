package com.hanli.simpnote.statistics;

import javafx.fxml.FXML;
import javafx.scene.control.Label;
import java.text.DecimalFormat;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class StatisticsViewController {

    @FXML
    private Label lblWordCount;

    @FXML
    private Label lblCharCountIncSpace;

    @FXML
    private Label lblCharCountExcSpace;

    @FXML
    private Label lblParaCount;

    private String targetArticle;

    private final DecimalFormat fmt = new DecimalFormat("#,###");

    @FXML
    void btnRefreshClicked() {
        Pattern pt = Pattern.compile("[ \\n]|[A-Za-z0-9'′\\-]+");
        int lineCount = 0, spaceCount = 0, wordCount = 0;
        Matcher mt = pt.matcher(targetArticle);

        // 重复查找空格或回车符直到文档结束
        while (mt.find()) {
            if (mt.group().matches("[A-Za-z0-9'′\\-]+"))
                wordCount++;
            else if (mt.group().equals(" "))
                spaceCount++;
            else
                lineCount++;
        }

        // 设置统计信息
        this.lblWordCount.setText(fmt.format(wordCount));
        this.lblCharCountExcSpace.setText(fmt.format(targetArticle.length()-spaceCount-lineCount));
        this.lblCharCountIncSpace.setText(fmt.format(targetArticle.length()-lineCount));
        this.lblParaCount.setText(fmt.format(lineCount+1));
    }

    public void setTargetArticle(String targetArticle) {
        this.targetArticle = targetArticle;
        this.btnRefreshClicked();
    }

}
