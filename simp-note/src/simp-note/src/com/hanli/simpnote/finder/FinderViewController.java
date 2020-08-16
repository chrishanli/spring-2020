package com.hanli.simpnote.finder;

import com.hanli.simpnote.notepad.Notepad;
import com.hanli.simpnote.utils.NotePadUtils;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.scene.control.*;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class FinderViewController {

    @FXML
    private TabPane tabFindReplace;

    @FXML
    private Button btnFindFind;

    @FXML
    private Button btnFindLast;

    @FXML
    private Button btnFindNext;

    @FXML
    private Label lblFindOrder;

    @FXML
    private TextArea txtFindKeywords;

    @FXML
    private CheckBox chkRegExp;

    @FXML
    private CheckBox chkIgnoreCases;

    @FXML
    private Button btnReplaceFind;

    @FXML
    private Button btnReplaceThis;

    @FXML
    private Button btnReplaceAll;

    @FXML
    private TextArea txtReplaceKeywords;

    @FXML
    private TextArea txtReplaceTo;

    @FXML
    private CheckBox chkReplaceRegExp;

    @FXML
    private CheckBox chkReplaceIgnoreCases;

    @FXML
    private Tab tabFind;

    @FXML
    private Tab tabReplace;

    // 当replace操作完成之后要做的事情
    private EventHandler<?> selectionChangedHandler;

    // 查找结果（每个结果是一个数组，第一个数字是第一个字符的起始位置，第二个数字是字符的长度）
    ArrayList<int[]> results = new ArrayList<>();
    private int findCurrentIndex = 0;
    private int replaceCurrentCharAt = 0;
    private Notepad targetNotePad = null;

    public void setOnSelectionMayChanged(EventHandler<ActionEvent> event) {
        this.selectionChangedHandler = event;
    }

    // 查找算法
    static private void find(String keywords, boolean isKeyWordsRegex, boolean isCasesIgnored, String document, ArrayList<int[]> findResults) {
        findResults.clear();
        // 查找
        Pattern exp;
        // 看情况，是否要求正则表达式及是否要求大小写敏感
        if (isKeyWordsRegex) {
            if (isCasesIgnored) {
                exp = Pattern.compile(keywords, Pattern.CASE_INSENSITIVE);
            } else {
                exp = Pattern.compile(keywords);
            }
        } else {
            if (isCasesIgnored) {
                exp = Pattern.compile(Pattern.quote(keywords), Pattern.CASE_INSENSITIVE);
            } else {
                exp = Pattern.compile(Pattern.quote(keywords));
            }
        }
        // 开始匹配
        Matcher mt = exp.matcher(document);
        while (mt.find()) {
            findResults.add(new int[]{mt.start(), mt.end()});
        }
        if (findResults.size() == 0) {
            NotePadUtils.reportInfo("Nothing found", "Nothing in this document and the keywords appears to be similar in shape.");
        }
    }

    // 设置目的文本框
    public void setTargetNotePad(Notepad targetNotePad) {
        this.targetNotePad = targetNotePad;
    }

    // 设置初始文本
    public void setInitString(String initString) {
        this.txtFindKeywords.setText(initString);
        this.txtReplaceKeywords.setText(initString);
    }

    @FXML
    void btnFindFindClicked(ActionEvent event) {
        String keywords = this.txtFindKeywords.getText();
        if (keywords.equals("")) {
            return;
        }
        // 查找
        String document = this.targetNotePad.getText();
        find(keywords, this.chkRegExp.isSelected(), this.chkIgnoreCases.isSelected(), document, this.results);

        // 统计结果
        if (results.size() == 0) {
            this.lblFindOrder.setText("0/0");
            this.btnFindLast.setDisable(true);
            this.btnFindNext.setDisable(true);
        } else {
            this.lblFindOrder.setText(String.format("%d/%d", 1, results.size()));
            // 选择第一个
            this.findCurrentIndex = 1;
            this.setCurrentSelect(true, results.get(this.findCurrentIndex -1));
            if (this.selectionChangedHandler != null) {
                this.selectionChangedHandler.handle(null);
            }
        }
    }

    @FXML
    void btnFindLastClicked(ActionEvent event) {
        // 选择上一个
        this.findCurrentIndex--;
        setCurrentSelect(true, results.get(this.findCurrentIndex -1));
    }

    @FXML
    void btnFindNextClicked(ActionEvent event) {
        // 选择下一个
        this.findCurrentIndex++;
        setCurrentSelect(true, results.get(this.findCurrentIndex -1));
    }

    // 设置当前选择（输入true表示查找，false表示替换）
    private void setCurrentSelect(boolean isFinding, int[] select) {
        // 设置选择
        this.targetNotePad.selectRange(select[0], select[1]);
        if (isFinding) {
            this.btnFindNext.setDisable(this.findCurrentIndex == this.results.size());
            this.btnFindLast.setDisable(this.findCurrentIndex == 1);
            // 设置显示
            this.lblFindOrder.setText(String.format("%d/%d",
                    this.findCurrentIndex,
                    this.results.size()));
            // 更新选择指示器
            if (this.selectionChangedHandler != null) {
                this.selectionChangedHandler.handle(null);
            }
        }
    }

    @FXML
    void btnReplaceAllClicked(ActionEvent event) {
        String keywords = this.txtReplaceKeywords.getText();
        String replaceTo = this.txtReplaceTo.getText();
        if (keywords.equals("")) {
            NotePadUtils.reportInfo("Keyword was empty", "Replace could not proceed unless keywords are given.");
            return;
        }
        // 选择所有
        String document = this.targetNotePad.getText();
        ArrayList<int[]> wordsResult = new ArrayList<>();
        find(keywords, this.chkReplaceRegExp.isSelected(), this.chkReplaceIgnoreCases.isSelected(), document, wordsResult);
        // 替换所有
        for (int i = wordsResult.size()-1; i >= 0; i--) {
            int[] w = wordsResult.get(i);
            this.targetNotePad.replaceText(w[0], w[1], replaceTo);
        }
        // 提示用户
        if (wordsResult.size() == 0) {
            NotePadUtils.reportInfo("Nothing changed", "Nothing changed because no keyword found in this document.");
        } else {
            NotePadUtils.reportInfo("Replacement over", String.format("Replaced %d keyword(s) successfully.", wordsResult.size()));
        }
        // 做完成replace之后要做的事情
        if (this.selectionChangedHandler != null) {
            this.selectionChangedHandler.handle(null);
        }
    }

    @FXML
    void btnReplaceClicked(ActionEvent event) {
        // 执行replace
        this.targetNotePad.replaceSelection(this.txtReplaceTo.getText());
        this.btnReplaceThis.setDisable(true);
    }

    @FXML
    void btnReplaceFindClicked(ActionEvent event) {
        String keywords = this.txtReplaceKeywords.getText();
        if (keywords.equals("")) {
            return;
        }
        // 查找每一个
        String document = this.targetNotePad.getText();
        Pattern exp;
        // 看情况，是否要求正则表达式及是否要求大小写敏感
        if (this.chkReplaceRegExp.isSelected()) {
            if (this.chkReplaceIgnoreCases.isSelected()) {
                exp = Pattern.compile(keywords, Pattern.CASE_INSENSITIVE);
            } else {
                exp = Pattern.compile(keywords);
            }
        } else {
            if (this.chkReplaceIgnoreCases.isSelected()) {
                exp = Pattern.compile(Pattern.quote(keywords), Pattern.CASE_INSENSITIVE);
            } else {
                exp = Pattern.compile(Pattern.quote(keywords));
            }
        }
        // 开始匹配
        Matcher mt = exp.matcher(document);
        if (mt.find(replaceCurrentCharAt)) {
            // 找到一个，就选择之
            setCurrentSelect(false, new int[]{mt.start(), mt.end()});
            replaceCurrentCharAt = mt.end();
            this.btnReplaceThis.setDisable(false);
        } else {
            // 查找结束
            NotePadUtils.reportInfo("Finder reached the end of the document.", "Press \"Find\" again to do re-search.");
            replaceCurrentCharAt = 0;
            this.btnReplaceThis.setDisable(true);
            this.btnReplaceAll.setDisable(false);
        }
    }
    
    @FXML
    void tabFindSelected() {

    }

    @FXML
    void tabReplaceSelected() {

    }

}
