package com.hanli.simpnote.local;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import java.util.ArrayList;

@XmlRootElement(name = "recentDocuments")
public class RecentDocuments {

    private ArrayList<RecentDocument> docs;

    public ArrayList<RecentDocument> getDocs() {
        return docs;
    }

    @XmlElement(name = "list")
    public void setDocs(ArrayList<RecentDocument> docs) {
        this.docs = docs;
    }
}
