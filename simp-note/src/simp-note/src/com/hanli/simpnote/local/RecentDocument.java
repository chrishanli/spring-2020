package com.hanli.simpnote.local;

import javax.xml.bind.annotation.XmlElement;

public class RecentDocument {
    private String fileName;
    private String fileAbsolutePath;

    public String getFileName() {
        return this.fileName;
    }

    @XmlElement(name = "fileName")
    public void setFileName(String fileName) {
        this.fileName = fileName;
    }

    public String getFileAbsolutePath() {
        return this.fileAbsolutePath;
    }

    @XmlElement(name = "fileAbsolutePath")
    public void setFileAbsolutePath(String absolutePath) {
        this.fileAbsolutePath = absolutePath;
    }
}