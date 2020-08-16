package com.hanli.simpnote.notepad;

import com.hanli.simpnote.SimpNoteMain;
import com.hanli.simpnote.about.AboutViewController;
import com.hanli.simpnote.finder.FinderWindow;
import com.hanli.simpnote.local.PreferencesWindow;
import com.hanli.simpnote.local.SysPreference;
import com.hanli.simpnote.statistics.StatisticsWindow;
import com.hanli.simpnote.utils.CharPref;
import com.hanli.simpnote.utils.NotePadUtils;
import de.jensd.fx.glyphs.fontawesome.FontAwesomeIcon;
import de.jensd.fx.glyphs.fontawesome.FontAwesomeIconView;
import javafx.beans.binding.Bindings;
import javafx.beans.binding.BooleanBinding;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.event.ActionEvent;
import javafx.scene.input.*;
import javafx.scene.layout.BorderPane;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.stage.FileChooser;
import javafx.stage.Modality;
import javafx.stage.Stage;
import java.io.*;

public class NotePadViewController {

    @FXML
    private BorderPane borderPane;

    @FXML
    private BorderPane basePane;

    @FXML
    private Label lblSel;

    @FXML
    private Label lblLn;

    @FXML
    private Label lblCol;

    @FXML
    private Label lblEncoding;

    @FXML
    private ToolBar statusBar;

    @FXML
    private Button btnToolbarPaste;

    @FXML
    private Button btnToolBarCopy;

    @FXML
    private Button btnToolBarCut;

    @FXML
    private Button btnToolBarOpen;

    @FXML
    private Button btnToolBarSave;

    @FXML
    private Button btnToolBarUndo;

    @FXML
    private Button btnToolBarRedo;

    @FXML
    private Button btnToolBarFindRep;

    @FXML
    private Button btnToolBarFonts;

    @FXML
    private Spinner<Integer> txtToolBarCharWeight;

    private IconLabelMenuItem btnCopy;

    private IconLabelMenuItem btnCut;

    private IconLabelMenuItem btnUndo;

    private CheckMenuItem btnAutoWrap;

    private IconLabelMenuItem btnSave;

    private IconLabelMenuItem btnRedo;

    private CheckMenuItem btnStatusBarEnabled;

    private Notepad notepad;

    // 标志：当前文件是否已经保存
    private final BooleanProperty isSaved = new SimpleBooleanProperty(false);

    // 当前文件的偏好设置
    SysPreference sp = NotePadUtils.getSysPreference();

    // 当前文件
    File currFile = null;

    private MenuBar genMenuBar() {
        // 文件菜单
        Menu mnuFile = new Menu("文件");
        IconLabelMenuItem btnNewDoc = new IconLabelMenuItem(null, "新建空白文档",
                new KeyCodeCombination(KeyCode.N, KeyCombination.SHORTCUT_DOWN),
                event1 -> btnNewDocClicked());
        IconLabelMenuItem btnNewFromClip = new IconLabelMenuItem(null, "从剪贴簿新建文档",
                new KeyCodeCombination(KeyCode.N, KeyCombination.SHIFT_DOWN, KeyCombination.SHORTCUT_DOWN),
                this::btnNewFromClipClicked);
        IconLabelMenuItem btnOpen = new IconLabelMenuItem(FontAwesomeIcon.FOLDER_OPEN, "打开...",
                new KeyCodeCombination(KeyCode.O, KeyCombination.SHORTCUT_DOWN),
                this::btnOpenClicked);
        this.btnSave = new IconLabelMenuItem(FontAwesomeIcon.SAVE, "保存...",
                        new KeyCodeCombination(KeyCode.S, KeyCombination.SHORTCUT_DOWN),
                        this::btnSaveClicked);
        IconLabelMenuItem btnSaveAs = new IconLabelMenuItem(null, "另存为...",
                new KeyCodeCombination(KeyCode.S, KeyCombination.SHIFT_DOWN, KeyCombination.SHORTCUT_DOWN),
                this::btnSaveAsClicked);
        IconLabelMenuItem btnPref = new IconLabelMenuItem(FontAwesomeIcon.GEARS, "系统偏好设置...",
                new KeyCodeCombination(KeyCode.COMMA, KeyCombination.SHORTCUT_DOWN),
                this::btnPreferenceClicked);
        IconLabelMenuItem btnClose = new IconLabelMenuItem(FontAwesomeIcon.CLOSE, "关闭文档",
                new KeyCodeCombination(KeyCode.W, KeyCombination.SHORTCUT_DOWN),
                this::btnCloseDocClicked);
        mnuFile.getItems().addAll(
                btnNewDoc, btnNewFromClip, btnOpen,
                new SeparatorMenuItem(),
                this.btnSave, btnSaveAs,
                new SeparatorMenuItem(),
                btnPref,
                new SeparatorMenuItem(),
                btnClose
        );

        // 编辑菜单
        Menu mnuEdit = new Menu("编辑");
        this.btnUndo = new IconLabelMenuItem(FontAwesomeIcon.ROTATE_LEFT, "撤销",
                new KeyCodeCombination(KeyCode.Z, KeyCombination.SHORTCUT_DOWN),
                event -> this.notepad.undo());
        this.btnRedo = new IconLabelMenuItem(FontAwesomeIcon.ROTATE_RIGHT, "重做",
                new KeyCodeCombination(KeyCode.Z, KeyCombination.SHORTCUT_DOWN, KeyCombination.SHIFT_DOWN),
                event -> this.notepad.redo());
        this.btnCut = new IconLabelMenuItem(FontAwesomeIcon.CUT, "剪下",
                new KeyCodeCombination(KeyCode.X, KeyCombination.SHORTCUT_DOWN),
                event -> cut());
        this.btnCopy = new IconLabelMenuItem(FontAwesomeIcon.COPY, "拷贝",
                new KeyCodeCombination(KeyCode.C, KeyCombination.SHORTCUT_DOWN),
                event -> copy());
        IconLabelMenuItem btnPaste = new IconLabelMenuItem(FontAwesomeIcon.PASTE, "贴上",
                new KeyCodeCombination(KeyCode.V, KeyCombination.SHORTCUT_DOWN),
                event -> paste());
        IconLabelMenuItem btnDel = new IconLabelMenuItem(null, "删除",
                new KeyCodeCombination(KeyCode.DELETE),
                this::btnDelClicked);
        IconLabelMenuItem btnFindOrReplace = new IconLabelMenuItem(FontAwesomeIcon.SEARCH, "查找及替换...",
                new KeyCodeCombination(KeyCode.F, KeyCombination.SHORTCUT_DOWN),
                this::btnFindRepClicked);
        IconLabelMenuItem btnSelAll = new IconLabelMenuItem(null, "全部选定",
                new KeyCodeCombination(KeyCode.A, KeyCombination.SHORTCUT_DOWN),
                this::btnSelAllClicked);

        mnuEdit.getItems().addAll(
                this.btnUndo, this.btnRedo,
                new SeparatorMenuItem(),
                this.btnCut, this.btnCopy, btnPaste, btnDel,
                new SeparatorMenuItem(),
                btnFindOrReplace,
                new SeparatorMenuItem(),
                btnSelAll
        );

        // 视图菜单
        Menu mnuView = new Menu("检视");
        this.btnStatusBarEnabled = new CheckMenuItem("显示状态栏");
        this.btnStatusBarEnabled.setOnAction(this::btnStatusBarEnableClicked);
        this.btnAutoWrap = new CheckMenuItem("自动换行");
        this.btnAutoWrap.setOnAction(this::btnAutoWrapClicked);
        mnuView.getItems().addAll(
                this.btnStatusBarEnabled,
                new SeparatorMenuItem(),
                this.btnAutoWrap
        );

        // 帮助菜单
        Menu mnuHelp = new Menu("帮助");
        IconLabelMenuItem btnSeeHelp = new IconLabelMenuItem(null, "显示帮助", this::btnSeeHelpClicked);
        IconLabelMenuItem btnAbout = new IconLabelMenuItem(null, "关于", this::btnAboutClicked);
        mnuHelp.getItems().addAll(btnSeeHelp, btnAbout);

        // 汇总菜单
        return new MenuBar(
                mnuFile, mnuEdit, mnuView, mnuHelp
        );
    }

    private void copy() {
        this.notepad.copy();
    }

    private void cut() {
        this.notepad.cut();
    }

    private void paste() {
        this.notepad.paste();
    }

    // 美化工具栏
    private void genToolBar() {
        this.btnToolBarUndo.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.ROTATE_LEFT));
        this.btnToolBarUndo.setOnAction(event -> this.notepad.undo());

        this.btnToolBarRedo.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.ROTATE_RIGHT));
        this.btnToolBarRedo.setOnAction(event -> this.notepad.redo());

        this.btnToolBarOpen.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.FOLDER_OPEN));
        this.btnToolBarOpen.setOnAction(this::btnOpenClicked);

        this.btnToolBarSave.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.SAVE));
        this.btnToolBarSave.setOnAction(this::btnSaveClicked);

        this.btnToolbarPaste.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.PASTE));
        this.btnToolbarPaste.setOnAction(event -> paste());

        this.btnToolBarCopy.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.COPY));
        this.btnToolBarCopy.setOnAction(event -> copy());

        this.btnToolBarCut.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.CUT));
        this.btnToolBarCut.setOnAction(event -> cut());

        this.btnToolBarFindRep.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.SEARCH));
        this.btnToolBarFonts.setGraphic(new FontAwesomeIconView(FontAwesomeIcon.FONT));
        this.btnToolBarFonts.setOnAction(this::btnSelectFontClicked);

        // 工具栏上的字体快速选择器
        SpinnerValueFactory<Integer> svf = new SpinnerValueFactory<Integer>() {

            // 减小字号
            @Override
            public void decrement(int steps) {
                this.setValue(this.getValue() - steps);
                update();
            }

            @Override
            public void increment(int steps) {
                this.setValue(this.getValue() + steps);
                update();
            }

            private void update() {
                Font font = sp.getCurrTextStyle().getFont();
                sp.getCurrTextStyle().setFont(
                        Font.font(font.getFamily(), this.getValue())
                );
                notepad.setStyle(sp.getCurrTextStyle());
            }
        };
        this.txtToolBarCharWeight.setValueFactory(svf);
        this.updateToolBarFonts();
    }


    // 更新工具栏上的字体选择器
    private void updateToolBarFonts() {
        this.btnToolBarFonts.setStyle("-fx-font-family: " + this.sp.getCurrTextStyle().getFont().getFamily());
        this.btnToolBarFonts.setText(this.sp.getCurrTextStyle().getFont().getName());
        this.txtToolBarCharWeight
                .getValueFactory()
                .setValue((int)sp.getCurrTextStyle().getFont().getSize());
    }

    @FXML
    void initialize() {
        // 新建系统菜单
        MenuBar menuBar = genMenuBar();
        if (NotePadUtils.isMacOS()) {
            menuBar.setUseSystemMenuBar(true);
            this.basePane.getChildren().add(menuBar);
        } else {
            this.basePane.setTop(menuBar);
        }

        if (sp.getCurrTextStyle() == null) {
            sp.setCurrFont(new CharPref());
            sp.getCurrTextStyle().setFont(Font.font("arial", FontWeight.NORMAL, 18));
        }

        // 美化工具栏
        genToolBar();

        // 新建一个文档区域
        this.notepad = new Notepad();
        this.notepad.setWrapText(sp.isAutoWrapEnabled());
        this.notepad.setStyle(sp.getCurrTextStyle());
        this.notepad.setOnKeyTyped(this::txtNotePad_KeyTyped);
        this.notepad.setOnInputMethodTextChanged(e -> {
            if (e.getCommitted().length() != 0) {
                this.notepad.insertText(this.notepad.getCaretPosition(), e.getCommitted());
            }
        });

        // 构建界面
        this.borderPane.setCenter(notepad);
        this.btnStatusBarEnabled.setSelected(sp.isStatusBarEnabled());
        this.btnAutoWrap.setSelected(sp.isAutoWrapEnabled());
        if (!sp.isStatusBarEnabled()) {
            this.borderPane.getChildren().remove(this.statusBar);
        }
        this.lblEncoding.setText(sp.getCurrOpenEncoding().toString());

        // 构建右键菜单
        genRightClickMenu();

        // 绑定
        BooleanBinding selectionNullProperty = Bindings.createBooleanBinding(
                () -> notepad.getSelection().getLength() == 0,
                notepad.selectionProperty()
        );
        // 1. 绑定撤销，重做按钮
        this.btnUndo.setDisable(true);
        this.btnToolBarUndo.setDisable(true);
        this.notepad.undoAvailableProperty().addListener((observable, oldValue, newValue) -> this.btnUndo.setDisable(!newValue));
        this.btnToolBarUndo.disableProperty().bind(this.btnUndo.disableProperty());

        this.btnRedo.setDisable(true);
        this.btnToolBarRedo.setDisable(true);
        this.notepad.redoAvailableProperty().addListener(((observable, oldValue, newValue) -> this.btnRedo.setDisable(!newValue)));
        this.btnToolBarRedo.disableProperty().bind(this.btnRedo.disableProperty());

        // 2. 绑定保存按钮
        this.btnToolBarCopy.disableProperty().bind(Bindings.createBooleanBinding(
                () -> btnSave.isDisable(), btnSave.disableProperty()
        ));
        this.btnSave.disableProperty().bind(isSaved);

        // 3. 绑定目前所在之行、列及选中文字长度之显示
        this.lblLn.textProperty().bind(Bindings.createStringBinding(
                () -> String.valueOf(notepad.getCurrentParagraph() + 1),
                notepad.getCaretSelectionBind().paragraphIndexProperty()
        ));
        this.lblCol.textProperty().bind(Bindings.createStringBinding(
                () -> String.valueOf(notepad.getCaretColumn()),
                notepad.getCaretSelectionBind().columnPositionProperty()
        ));
        this.lblSel.textProperty().bind(Bindings.createStringBinding(
                () -> String.valueOf(notepad.getSelection().getLength()),
                notepad.selectionProperty()
        ));

        // 4. 绑定复制、剪切按钮
        this.btnToolBarCopy.disableProperty().bind(selectionNullProperty);
        this.btnCopy.disableProperty().bind(selectionNullProperty);
        this.btnToolBarCut.disableProperty().bind(selectionNullProperty);
        this.btnCut.disableProperty().bind(selectionNullProperty);
    }

    // 设置右键菜单
    private void genRightClickMenu() {
        // 新建各种按钮
        IconLabelMenuItem copyItem = new IconLabelMenuItem(FontAwesomeIcon.COPY, "复制",
                new KeyCodeCombination(KeyCode.C, KeyCombination.SHORTCUT_DOWN),
                event -> this.notepad.copy());
        IconLabelMenuItem cutItem = new IconLabelMenuItem(FontAwesomeIcon.CUT, "剪下",
                new KeyCodeCombination(KeyCode.X, KeyCombination.SHORTCUT_DOWN),
                event -> this.notepad.cut());
        IconLabelMenuItem pasteItem = new IconLabelMenuItem(FontAwesomeIcon.PASTE, "贴上",
                new KeyCodeCombination(KeyCode.P, KeyCombination.SHORTCUT_DOWN),
                event -> this.notepad.paste());
        IconLabelMenuItem searchThis = new IconLabelMenuItem(FontAwesomeIcon.SEARCH, "",
                new KeyCodeCombination(KeyCode.S, KeyCombination.SHORTCUT_DOWN, KeyCombination.ALT_DOWN),
                event -> {
                    FinderWindow fw = new FinderWindow(this.notepad, this.notepad.getSelectedText());
                    fw.show(completionEvent -> {
                    });
                });
        IconLabelMenuItem chooseFont = new IconLabelMenuItem(FontAwesomeIcon.FONT, "字体...",
                this::btnSelectFontClicked);
        IconLabelMenuItem statisticsHere = new IconLabelMenuItem(FontAwesomeIcon.BAR_CHART, "检视统计数据...",
                event -> {
                    StatisticsWindow sw = new StatisticsWindow(this.notepad.getSelectedText());
                    try {
                        sw.show();
                    } catch (IOException e) {
                        NotePadUtils.reportError("SimpNote现时不可用", "An error occurred: "+e);
                    }
                });
        IconLabelMenuItem copyFilePath = new IconLabelMenuItem(null, "拷贝当前文件路径",
                event -> {
                    if (this.currFile != null) {
                        String filePath = this.currFile.getAbsolutePath();
                        NotePadUtils.appendToSysClipboardText(filePath);
                    }
                });

        // 设置右键菜单满足要求才能允许相关按钮的点按事件
        BooleanBinding selectionNullProperty = Bindings.createBooleanBinding(
                () -> notepad.getSelection().getLength() == 0,
                notepad.selectionProperty()
        );
        cutItem.disableProperty().bind(selectionNullProperty);
        copyItem.disableProperty().bind(selectionNullProperty);
        searchThis.disableProperty().bind(selectionNullProperty);
        statisticsHere.disableProperty().bind(selectionNullProperty);
        this.notepad.setOnContextMenuRequested(event -> {
            int length = notepad.getSelection().getLength();
            String findWords;
            if (length == 0) {
                findWords = "查找及替换当前选定之区域..";
            } else if (length <= 16) {
                findWords = "在当前文档中查找及替换\"" + notepad.getSelectedText() + "\"...";
            } else {
                findWords = "在当前文档中查找及替换\"" + notepad.getSelectedText().substring(0, 16) + "..." + "\"...";
            }
            searchThis.setText(findWords);
            copyFilePath.setDisable(this.currFile == null);
        });

        ContextMenu cm = new ContextMenu();
        cm.getItems().addAll(
                copyItem,
                cutItem,
                pasteItem,
                new SeparatorMenuItem(),
                searchThis,
                chooseFont,
                statisticsHere,
                new SeparatorMenuItem(),
                copyFilePath
        );

        this.notepad.setContextMenu(cm);
    }

    // 显示一个已有的文件（已经有文件）
    public void setDisplay(File infile) {
        FileInputStream in;
        try {
            in = new FileInputStream(infile);
        } catch (IOException e) {
            NotePadUtils.reportError("SimpNote现时不可用", "Unknown error: " + e.getLocalizedMessage());
            return;
        }
        // 创建一个缓冲区，将文件全部写入进来以便显示
        byte[] buff = new byte[(int)infile.length()];
        try {
            int readBytes = in.read(buff);
            if (readBytes < 0) {
                throw new IOException("Read out file failed");
            }
        } catch (IOException e) {
            NotePadUtils.reportError("SimpNote现时不可用", "Unknown error: " + e.getLocalizedMessage());
            return;
        }

        // 设置显示
        try {
            this.setDisplay(new String(buff, sp.getCurrOpenEncoding().getSysName()));
        } catch (UnsupportedEncodingException e) {
            NotePadUtils.reportError("未支持的编码方式："+sp.toString(), "Unsupported encoding exception: "+e);
            this.setDisplay("");
        }
        this.isSaved.set(true);

        // 设置文件
        currFile = infile;
    }

    // 显示初始字符串（未有保存文件）
    public void setDisplay(String initialString) {
        this.notepad.appendText(initialString);
        this.notepad.getUndoManager().forgetHistory();
    }

    private void setAutoWrapEnabled(boolean isEnabled) {
        this.sp.setAutoWrapEnabled(isEnabled);
        this.notepad.setWrapText(isEnabled);
    }

    private void setStatusBarEnabled(boolean isEnabled) {
        if (!this.sp.isStatusBarEnabled() && isEnabled) {
            this.borderPane.setBottom(this.statusBar);
        } else if (this.sp.isStatusBarEnabled() && !isEnabled) {
            this.borderPane.getChildren().remove(this.statusBar);
        }
        this.sp.setStatusBarEnabled(isEnabled);
    }


    @FXML
    void btnNewDocClicked() {
        // 新建newDoc窗口
        NotePadWindow notePadWindow = new NotePadWindow("");
        notePadWindow.show();
    }

    @FXML
    void btnNewFromClipClicked(ActionEvent event) {
        // 获取剪贴板内容
        String clpBrdMatter = NotePadUtils.getSysClipboardText();
        // 新建newDoc窗口
        NotePadWindow notePadWindow = new NotePadWindow(clpBrdMatter);
        notePadWindow.show();
    }

    @FXML
    void btnOpenClicked(ActionEvent event) {
        Stage stage = (Stage)this.notepad.getScene().getWindow(); // 获取当前窗口
        // 用打开文件新建newDoc窗口
        NotePadUtils.makeOpenDocument(stage);
    }

    // 显示对话框，以保存一个文件
    private void saveFile(FileChooser saveFile) {
        File outfile = saveFile.showSaveDialog(this.notepad.getScene().getWindow());
        if (outfile != null) {
            if (sp.isTxtNeeded()) {
                String newOutFilePath = outfile.getAbsolutePath() + ".txt";
                outfile = new File(newOutFilePath);
            }
            forceSaveToFile(outfile, sp.getCurrSaveEncoding().getSysName());
            // 记载进数据库
            NotePadUtils.addRecentDoc(outfile);
        }
    }

    // 强制保存至某一文件
    private void forceSaveToFile(File outfile, String encoding) {
        if (outfile != null) {
            // 保存内容至该文件中
            if (NotePadUtils.saveText(this.notepad.getText(), outfile, encoding)) {
                // 设置已经保存
                this.isSaved.set(true);
                System.out.println("Wrote at: " + outfile.getAbsolutePath());

                // 设置标题
                Stage currStage = (Stage) this.notepad.getScene().getWindow();
                currStage.setTitle(outfile.getName() + " - SimpNote");
                currFile = outfile;
            } else {
                currFile = null;
            }
        }
    }

    @FXML // 点击了"保存文件"
    void btnSaveClicked(ActionEvent event) {
        // 如果是之前从未保存过当前文件
        if (currFile == null) {
            FileChooser saveFile = new FileChooser();
            saveFile.setTitle("保存");
            saveFile.setTitle("未命名");
            saveFile(saveFile);
        } else {
            // 如果之前保存过文件了，就直接写入
            if (NotePadUtils.saveText(this.notepad.getText(), currFile, sp.getCurrSaveEncoding().getSysName())) {
                // 设置已经保存
                this.isSaved.set(true);
            }
        }
    }

    @FXML
    void btnSaveAsClicked(ActionEvent event) {
        // 另存为
        FileChooser saveFile = new FileChooser();
        saveFile.setTitle("另存为");
        if (currFile == null) {
            saveFile.setInitialFileName("未命名");
        } else {
            saveFile.setInitialFileName(currFile.getName());
        }

        saveFile(saveFile);
    }

    @FXML
    void btnPreferenceClicked(ActionEvent event) {
        PreferencesWindow prefW = new PreferencesWindow();
        try {
            prefW.setSysPreference(sp);
            prefW.show();
        } catch (IOException e) {
            NotePadUtils.reportError("SimpNote现时不可用",
                    "Unknown error: " + e.getLocalizedMessage());
        }

        // 设置状况
        if (prefW.isSaved()) {
            updateFilePref(sp);
        }
    }

    // 刷新可视的设置
    private void updateFilePref(SysPreference fp) {
        this.notepad.setStyle(fp.getCurrTextStyle());
        this.lblEncoding.setText(sp.getCurrOpenEncoding().toString());
        this.updateToolBarFonts();
    }

    void btnSelectFontClicked(ActionEvent ae) {
        // 显示字体、颜色窗口
        CharPref charPref = NotePadUtils.changeCharPref(true);
        if (charPref != null) {
            // 非空，则设置该字体
            this.sp.setCurrFont(charPref);
            this.updateFilePref(sp);
        }
    }

    void btnCloseDocClicked(ActionEvent ae) {
        if (!isSaved.get()) {
            String currFileName = currFile == null ? "未命名" : currFile.getName();
            if (!NotePadUtils.askUser("确认退出", "是否在未保存 \""
                    + currFileName + "\" 的情况下关闭该文档？")) {
                // 用户说：我还是别退出了
                return;
            }
        }
        // 显示初始界面及关闭窗口
        Stage stage = (Stage) this.notepad.getScene().getWindow();
        stage.close();
        SimpNoteMain.showInitPage();
    }

    @FXML
    void btnDelClicked(ActionEvent event) {
        // 删除
        this.notepad.replaceSelection("");
    }

    @FXML
    void btnSeeDocStatisticsClicked() {
        StatisticsWindow sw = new StatisticsWindow(this.notepad);
        try {
            sw.show();
        } catch (IOException e) {
            NotePadUtils.reportError("SimpNote现时不可用", "An error occurred: "+e);
        }
    }

    @FXML
    void btnFindRepClicked(ActionEvent event) {
        FinderWindow fw = new FinderWindow(this.notepad);
        fw.show(selectionChangedEvent -> {
        });
    }

    @FXML
    void btnSelAllClicked(ActionEvent event) {
        this.notepad.selectAll();
    }

    @FXML
    void btnAutoWrapClicked(ActionEvent event) {
        setAutoWrapEnabled(!sp.isAutoWrapEnabled());
    }

    @FXML
    void btnStatusBarEnableClicked(ActionEvent event) {
        setStatusBarEnabled(!sp.isStatusBarEnabled());
    }

    @FXML
    void btnSeeHelpClicked(ActionEvent event) {
        btnAboutClicked(null);
    }

    @FXML
    void btnAboutClicked(ActionEvent event) {
        // 窗体配置
        FXMLLoader loader = new FXMLLoader(AboutViewController.class.getResource("aboutView.fxml"));
        Parent rootView;
        try {
            rootView = loader.load();
        } catch (Exception e) {
            e.printStackTrace();
            NotePadUtils.reportError("SimpNote现时不可用", "Unknown error: " + e.toString());
            return;
        }
        Scene sc = new Scene(rootView);
        Stage window = new Stage();
        window.initModality(Modality.APPLICATION_MODAL);
        window.setScene(sc);
        window.setResizable(false);
        window.setTitle("关于 - SimpNote");
        window.showAndWait();
    }

    void txtNotePad_KeyTyped(KeyEvent ke) {
        this.isSaved.set(false);
    }

}
