/*
Launchy: Application Launcher
Copyright (C) 2007  Josh Karlin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CATALOG_H
#define CATALOG_H

#include <config.h>
#include <globals.h>
#include <bmapi.h>


#if defined(CATALOG_DLL)
#define CATALOG_DLL_CLASS_EXPORT __declspec(dllexport)
#else
#define CATALOG_DLL_CLASS_EXPORT __declspec(dllimport)
//#define CATALOG_DLL_CLASS_EXPORT 
#endif
/** 
\brief CatItem (Catalog Item) stores a single item in the index
*/



enum CATITEM_ITEM{
	CATITEM_FULLPATH=0,
	CATITEM_SHORTNAME,	
	CATITEM_COMEFROM,
	CATITEM_MAX
};
#define COPY_CATITEM(x) do{\
	fullPath = (x).fullPath;\
	shortName = (x).shortName;\
	lowName = (x).lowName;\
	icon = (x).icon;\
	usage = (x).usage;\
	time = (x).time;\
	hash_id = (x).hash_id;\
	comeFrom=(x).comeFrom;\
	isHasPinyin=(x).isHasPinyin;\
	pinyinReg=(x).pinyinReg;\
	type=(x).type;\
	allchars=(x).allchars;\
	alias2=(x).alias2;\
	domain = (x).domain;\
	groupId = (x).groupId;\
	parentId = (x).parentId;\
	shortCut=(x).shortCut;\
	delId=(x).delId;\
	args=(x).args;\
	idInTable=(x).idInTable;\
	pos = (x).pos;\
	realname = (x).realname;\
	}while(0);

class CATALOG_DLL_CLASS_EXPORT CatItem {
public:

	/** The full path of the indexed item */
	QString fullPath;
	/** The abbreviated name of the indexed item */
	QString shortName;
	/** The lowercase name of the indexed item */
	QString lowName;
	/** A path to an icon for the item */
	QString icon;
	QString args;
	QString pinyinReg;
	QString allchars;
	/* for link file*/
	QString realname;
	QString alias2;
	QString domain;
	/** How many times this item has been called by the user */
	uint usage;
	/** This is unused, and meant for plugin writers and future extensions */
	uint time;
	/** The plugin id of the creator of this CatItem */
	uint hash_id;
	uint delId;
	uint idInTable;
	uint pos;//record the postion when isHasPinyin  true

	uint groupId;
	uint parentId;
	/*is has pingyin*/
	unsigned char isHasPinyin;
	unsigned char comeFrom;
	unsigned char shortCut;
	unsigned char type;//0--file or url 1--dir
	//unsigned short hanziNums;
	/*pinyin depth*/
	//unsigned int pinyinDepth;
	/*pinyin reg*/



	CatItem() {}

	CatItem(QString full,int flag,bool isDir=false) ;
	CatItem(const QString& full,  const QString& shortN,const QString& realName) ;
	CatItem(QString full, QString shortN,int flag) ;

	CatItem(QString full, QString shortN, uint i_d,int flag)  ;

	CatItem(QString full, QString shortN,QString arg, int flag) ;

	/** This is the constructor most used by plugins 
	\param full The full path of the file to execute
	\param The abbreviated name for the entry
	\param i_d Your plugin id (0 for Launchy itself)
	\param iconPath The path to the icon for this entry
	\warning It is usually a good idea to append ".your_plugin_name" to the end of the full parameter
	so that there are not multiple items in the index with the same full path.
	*/
	CatItem(QString full, QString shortN, uint i_d, QString iconPath,int flag) ;
	CatItem(QString full, QString shortN,QString arg, uint i_d, QString iconPath,int flag);

	CatItem(const CatItem &s);
	void getPinyinReg(const QString& str);
	static void prepareInsertQuery(QSqlQuery* q,const CatItem& item,int tableid=0);
	static void addCatitemToDb(QSqlDatabase* db,CatItem& item);
	static void modifyCatitemFromDb(QSqlDatabase *db,CatItem& item,uint index);
	static void deleteCatitemFromDb(QSqlDatabase *db,CatItem& item,uint index);
	static void importNetworkBookmark(QSqlDatabase *db,QList < bookmark_catagory > *s,int groupid);
	CatItem& operator=( const CatItem &s ) {
		COPY_CATITEM(s);		
		return *this;
	}

	bool operator==(const CatItem& other) const{
		if (fullPath == other.fullPath)
			return true;
		return false;
	}

};


/** InputData shows one segment (between tabs) of a user's query 
A user's query is typically represented by List<InputData>
and each element of the list represents a segment of the query.

E.g.  query = "google <tab> this is my search" will have 2 InputData segments
in the list.  One for "google" and one for "this is my search"
*/
class CATALOG_DLL_CLASS_EXPORT InputData 
{
private:
	/** The user's entry */
	QString text;
	/** Any assigned labels to this query segment */
	QSet<uint> labels;
	/** A pointer to the best catalog match for this segment of the query */
	CatItem topResult;
	/** The plugin id of this query's owner */
	uint id;
public:
	/** Get the labels applied to this query segment */
	QSet<uint>  getLabels() { return labels; }
	/** Apply a label to this query segment */
	void setLabel(uint l) { labels.insert(l); }
	/** Check if it has the given label applied to it */
	bool hasLabel(uint l) { return labels.contains(l); }

	/** Set the id of this query

	This can be used to override the owner of the selected catalog item, so that 
	no matter what item is chosen from the catalog, the given plugin will be the one
	to execute it.

	\param i The plugin id of the plugin to execute the query's best match from the catalog
	*/
	void setID(uint i) { id = i; }

	/** Returns the current owner id of the query */
	uint getID() { return id; }

	/** Get the text of the query segment */
	QString  getText() { return text; }

	/** Set the text of the query segment */
	void setText(QString t) { text = t; }

	/** Get a pointer to the best catalog match for this segment of the query */
	CatItem&  getTopResult() { return topResult; }

	/** Change the best catalog match for this segment */
	void setTopResult(CatItem sr) { topResult = sr; }

	InputData() { id = 0; }
	InputData(QString str) : text(str) { id = 0;}
};
//extern QString searchTxt;
//CATALOG_DLL_CLASS_EXPORT bool  CatLess (CatItem* left, CatItem* right); 
//CATALOG_DLL_CLASS_EXPORT bool  CatLessNoPtr (CatItem & a, CatItem & b);

inline QDataStream &operator<<(QDataStream &out, const CatItem &item) {
	out << item.fullPath;
	out << item.shortName;
	out << item.lowName;
	out << item.icon;
	out << item.usage;
	out << item.time;
	out << item.hash_id;
	out << item.isHasPinyin;
	out << item.comeFrom;
	out << item.pinyinReg;
	out << item.allchars;
	out << item.alias2;
	out << item.domain;
	out << item.type;
	out << item.groupId;
	out << item.parentId;
	out << item.shortCut;
	out << item.delId;
	out << item.args;
	out << item.idInTable;
	out << item.pos;
	out << item.realname;
	return out;
}

inline QDataStream &operator>>(QDataStream &in, CatItem &item) {
	in >> item.fullPath;
	in >> item.shortName;
	in >> item.lowName;
	in >> item.icon;
	in >> item.usage;
	in >> item.time;
	in >> item.hash_id;
	in >> item.isHasPinyin;
	in >> item.comeFrom;
	in >> item.pinyinReg;
	in >> item.allchars;
	in >> item.alias2;
	in >> item.domain;
	in >> item.type;
	in >> item.groupId;
	in >> item.parentId;
	in >> item.shortCut;
	in >> item.delId;
	in >> item.args;
	in >> item.idInTable;
	in >> item.pos;
	in >> item.realname;
	return in;
}




#endif
