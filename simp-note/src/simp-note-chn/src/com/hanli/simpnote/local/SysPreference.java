package com.hanli.simpnote.local;

import com.hanli.simpnote.utils.NotePadUtils;
import com.hanli.simpnote.utils.CharPref;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlTransient;

@XmlRootElement(name = "pref")
public class SysPreference {

    // 标志：是否自动换行
    private boolean isAutoWrapEnabled = false;

    // 字体
    private CharPref currFont = null;

    // 当前保存文件编码方式
    private NotePadUtils.Encoding currSaveEncoding = NotePadUtils.Encoding.UTF8;

    // 当前打开文件编码方式
    private NotePadUtils.Encoding currOpenEncoding = NotePadUtils.Encoding.UTF8;

    // 标志：是否需要状态栏
    private boolean isStatusBarEnabled = true;


    // 当前是否需要文件尾部添加一个.txt
    private boolean isTxtNeeded;

    public boolean isAutoWrapEnabled() {
        return isAutoWrapEnabled;
    }

    @XmlElement(name = "autoWrapEnabled")
    public void setAutoWrapEnabled(boolean autoWrapEnabled) {
        isAutoWrapEnabled = autoWrapEnabled;
    }

    public boolean isStatusBarEnabled() {
        return isStatusBarEnabled;
    }

    @XmlElement(name = "statusBarEnabled")
    public void setStatusBarEnabled(boolean statusBarEnabled) {
        isStatusBarEnabled = statusBarEnabled;
    }

    public CharPref getCurrTextStyle() {
        return currFont;
    }

    @XmlTransient
    public void setCurrFont(CharPref currFont) {
        this.currFont = currFont;
    }

    public NotePadUtils.Encoding getCurrSaveEncoding() {
        return currSaveEncoding;
    }

    @XmlElement(name = "saveEncoding")
    public void setCurrSaveEncoding(NotePadUtils.Encoding currSaveEncoding) {
        this.currSaveEncoding = currSaveEncoding;
    }

    public NotePadUtils.Encoding getCurrOpenEncoding() {
        return currOpenEncoding;
    }

    @XmlElement(name = "openEncoding")
    public void setCurrOpenEncoding(NotePadUtils.Encoding currOpenEncoding) {
        this.currOpenEncoding = currOpenEncoding;
    }

    public boolean isTxtNeeded() {
        return isTxtNeeded;
    }

    @XmlElement(name = "txtExNeeded")
    public void setTxtNeeded(boolean txtNeeded) {
        isTxtNeeded = txtNeeded;
    }
}
