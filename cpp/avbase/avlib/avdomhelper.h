///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  DI Alexander Wemmer, a.wemmer@avibit.com
    \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
    \brief
*/

#ifndef __AVDOMHELPER_H__
#define __AVDOMHELPER_H__

#include <QDomElement>
#include <QString>
#include <QPolygon>

#include "avlib_export.h"
#include "avlog.h"

#define ATTR_QRECT_LEFT   "left"
#define ATTR_QRECT_RIGHT  "right"
#define ATTR_QRECT_TOP    "top"
#define ATTR_QRECT_BOTTOM "bottom"
#define ATTR_QSIZE_WIDTH  "width"
#define ATTR_QSIZE_HEIGHT "height"
#define ATTR_QPOINT_X     "x"
#define ATTR_QPOINT_Y     "y"
#define ATTR_QPOINTF_X    "x"
#define ATTR_QPOINTF_Y    "y"
#define CHILD_QPOLYGON_POINT          "point"
#define CHILD_QPOINTFPOLYGON_POINT    "point"

/////////////////////////////////////////////////////////////////////////////

//! XML DOM helper methods
class AVLIB_EXPORT AVDOMHelper
{
public:
    //! Returns the XML representation of the node and all its children
    //! \param ident the amount of space to indent the node.
    //! \sa QDomNode::save()
    static QString printNode(const QDomNode& node, int ident);

    //! parses the value of the attribute with the given name from the given
    //! DOM element and assigns the converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseInt32AttributeFromElement(const QDomElement& element,
                                               const QString& attribute_name,
                                               qint32& value);

    //! parses the value of the attribute with the given name from the given
    //! DOM element and assigns the converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseStringAttributeFromElement(const QDomElement& element,
                                                const QString& attribute_name,
                                                QString& value);

    //! parses the value of the attribute with the given name from the given
    //! DOM element and assigns the converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseDoubleAttributeFromElement(const QDomElement& element,
                                                const QString& attribute_name,
                                                double& value);

    //! parses the content of the given DOM element and assigns the
    //! converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseInt32ContentFromElement(const QDomElement& element,
                                             qint32& value);

    //! parses the content of the given DOM element and assigns the
    //! converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseUInt32ContentFromElement(const QDomElement& element,
                                              quint32& value);

    //! parses the content of the given DOM element and assigns the
    //! converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseBoolContentFromElement(const QDomElement& element,
                                            bool& value);
    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseStringContentFromElement(const QDomElement& element,
                                              QString& value);

    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on successive, false otherwise.
    static bool parseDoubleContentFromElement(const QDomElement& element,
                                              double& value);

    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on success, false otherwise.
    static bool parseQRectContentFromElement(const QDomElement& element,
                                              QRect& value);

    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on success, false otherwise.
    static bool parseQPointContentFromElement(const QDomElement& element,
                                              QPoint& value);

    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on success, false otherwise.
    static bool parseQPointFContentFromElement(const QDomElement& element,
                                              QPointF& value);

    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on success, false otherwise.
    static bool parseQSizeContentFromElement(const QDomElement& element,
                                             QSize& value);

    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on success, false otherwise.
    static bool parseQPolygonContentFromElement(const QDomElement& element,
                                                QPolygon &value);

    //! parses the content of the given element and assigns the
    //! converted value to the given one.
    //! Will return true on success, false otherwise.
    static bool parseQPointFPolygonContentFromElement(const QDomElement& element,
                                                      QVector<QPointF> &value);

    //! Creates a new QDomElement with the given value as content
    static QDomElement createBoolElement(QDomDocument &document,const QString &name,
                                         bool value);

    //! Creates a new QDomElement with the given value as content
    static QDomElement createDoubleElement(QDomDocument &document,const QString &name,
                                           qreal value);

    //! Creates a new QDomElement with the given value as attributes
    static QDomElement createQPointElement(QDomDocument &document,const QString &name,
                                           const QPoint &value);

    //! Creates a new QDomElement with the given value as attributes
    static QDomElement createQPointFElement(QDomDocument &document,const QString &name,
                                           const QPointF &value);

    //! Creates a new QDomElement with the given value as attributes
    static QDomElement createQPolygonElement(QDomDocument &document,const QString &name,
                                             const QPolygon &value);

    //! Creates a new QDomElement with the given value as attributes
    static QDomElement createQPointFPolygonElement(QDomDocument &document,const QString &name,
                                                   const QVector<QPointF> &value);

    //! Creates a new attribute of the element with the given name and value
    static void createDoubleAttribute(QDomElement &element,const QString &name,
                                             double value);

};


#endif /* __AVDOMHELPER_H__ */

// End of file
