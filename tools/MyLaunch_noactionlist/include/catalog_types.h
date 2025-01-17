#ifndef CATALOG_TYPE_H_
#define CATALOG_TYPE_H_

#include <config.h>
#include <globals.h>
#include <catalog.h>

/** This class does not pertain to plugins */
#if defined(CATALOG_TYPES_DLL)
#define CATALOG_TYPES_DLL_CLASS_EXPORT __declspec(dllexport)
#else
#define CATALOG_TYPES_DLL_CLASS_EXPORT __declspec(dllimport)
//#define CATALOG_TYPES_DLL_CLASS_EXPORT 
#endif
class CATALOG_TYPES_DLL_CLASS_EXPORT Catalog {
public:
	Catalog(QSettings* setting,CatItem* searchResult,QSqlDatabase* b) {
		settings=setting;
		searchResults=searchResult;
		db=b;
	}
	virtual ~Catalog() {}
	virtual void addItem(CatItem& item,int comefrom,uint delId) = 0;
	virtual void clearItem() = 0;
	virtual int count() = 0;
	virtual const CatItem & getItem(int) = 0;
	//bool matches(CatItem* item, QString& txt);
	//void pinyinMatches(QStringList& strlist,int i,int max,QString e_s,QString &txt,bool& ret);
	//void pinyinMatchesEx(QStringList& strlist,QString &txt,bool& ret,bool CaseSensitive);
	void searchCatalogs(QString, QList<CatItem*> & );
	//	virtual void incrementUsage(const CatItem& item) = 0;
	virtual int getUsage(const QString& path) = 0;
	void checkHistory(QString txt, QList<CatItem*> & list);	
	void getHistory(QList < CatItem *> &out);
private:	
	virtual QList<CatItem*> search(QString) = 0;
public:
	//QString searchText;
	QSettings* settings;
	CatItem* searchResults;
	QSqlDatabase* db;
};

#if 0
/** This class does not pertain to plugins */
// The fast catalog searches quickly but 
// addition of items is slow and uses a lot of memory
class CATALOG_TYPES_DLL_CLASS_EXPORT FastCatalog : public Catalog {
private:
	QVector<CatItem> catList;
	//	QList<CatItem> catList;
	QHash<QChar, QList<CatItem*> > catIndex;
public:
	FastCatalog(QSettings* setting) : Catalog(setting) {}
	void addItem(CatItem item);
	QList<CatItem*> search(QString);
	int count() { return catList.count(); }
	const CatItem & getItem(int i) { 
		return catList[i];
	}
	void incrementUsage(const CatItem& item);
	int getUsage(const QString& path);
};
#endif
/** This class does not pertain to plugins */
// The slow catalog searches slowly but
// adding items is fast and uses less memory
// than FastCatalog

class CATALOG_TYPES_DLL_CLASS_EXPORT  SlowCatalog : public Catalog {
private:
	QVector<CatItem> catList;
	//	QList<CatItem> catList;
public:
	SlowCatalog(QSettings* setting,CatItem* searchResult,QSqlDatabase* db) : Catalog(setting,searchResult,db) {}
	void addItem(CatItem& item,int comefrom,uint delId);
	void clearItem() ;
	QList<CatItem*> search(QString);
	int count() { return catList.count(); }
	const CatItem & getItem(int i){return catList[i];}
	//	void incrementUsage(const CatItem& item);
	int getUsage(const QString& path);
	bool pinyinsearch(const QStringList& list,const int size,int pos,const QString& searchtxt,const int ssize,int& depth,QString suffix);
	int isAllIn(const QString& src,const QString& all);
	//int debugon;
	//	uint isExistInDb(CatItem &item);
};
#endif
