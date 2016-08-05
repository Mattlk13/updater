#ifndef __LOADQM_H__
#define __LOADQM_H__

#include "loadable.h"

class LoadQm : public Loadable
{
public:
	LoadQm(const QString &name, const int grade = 0,
		const bool system = false, const QString &comment = QString::null,
		const QString &filename = QString::null);

	LoadQm(const QDomElement &, const bool system, QStringList &, QList<bool> &);

	virtual int writeToDB(const QByteArray &, const QString pkgname, QString &);
};

#endif
