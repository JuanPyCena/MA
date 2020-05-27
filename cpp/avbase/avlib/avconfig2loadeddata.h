///////////////////////////////////////////////////////////////////////////////
//
// Package:   AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright: AviBit data processing GmbH, 2001-2013
//
// Module:    AVLIB - AviBit Library
//
///////////////////////////////////////////////////////////////////////////////

/*! \file
    \author  Christian Muschick, c.muschick@avibit.com
    \author  QT4-PORT: Christian Muschick, c.muschick@avibit.com
    \brief   Classes to handle config parameters which can be freely partitioned among config files.
 */

#ifndef AVCONFIG2LOADEDDATA_INCLUDED
#define AVCONFIG2LOADEDDATA_INCLUDED

#include "avlib_export.h"
#include "avconfig2interfaces.h"

#include "avlog.h"
#include <set>

///////////////////////////////////////////////////////////////////////////////
/**
 * @brief The AVConfig2LoadedData class provides a mechanism to store avconfig2 elements.
 *
 * This class is also used internally by the AVConfig2Container to store its data.
 * It provides access by index (either per element type, or per total index), and optionally access by name (then
 * the elements are kept unique; if an element with an identical name is added, it replaces any existing element).
 * Names are not unique across element types, so access by name happens for a specific element type.
 *
 * The interface is template-based to avoid accessors for all different AVConfig2 element types.
 * Usage e.g.:
 * \verbatim
 * uint c = data.getElementCount<AVConfig2ImporterClient::LoadedParameter>();
 * \endverbatim
 *
 * For saving, refer to AVConfig2SimpleSaver.
 */
class AVLIB_EXPORT AVConfig2LoadedData : public AVConfig2ImporterClient
{
    friend class AVConfig2Test;
public:

    //! SubconfigTemplateSpecification is used for subconfig templates API in the
    //! AVConfig2LoadedData (e.g. instantiateSubconfigTemplate, listSubconfigTemplates)
    //! It stores the name and location of the subconfig template.
    class AVLIB_EXPORT SubconfigTemplateSpecification
    {
    public:
        SubconfigTemplateSpecification();
        SubconfigTemplateSpecification(const QString& template_name, const AVConfig2StorageLocation& location);

        bool operator==(const SubconfigTemplateSpecification& other) const;

        void setTemplateName(const QString& template_name);
        void setLocation(const AVConfig2StorageLocation& location);

        QString getTemplateName() const;
        AVConfig2StorageLocation getLocation() const;

    private:
        QString                  m_template_name;
        AVConfig2StorageLocation m_location;
    };

    /**
     * @param unique_elements If this is false, access by name is not supported (use case e.g. parameter editor).
     *                        If it is true, adding an element multiple times will modify the existing element.
     */
    AVConfig2LoadedData(bool unique_elements);
    AVConfig2LoadedData(const AVConfig2LoadedData& other);
    ~AVConfig2LoadedData() override;
    bool operator==(const AVConfig2LoadedData& other) const;
    AVConfig2LoadedData& operator=(const AVConfig2LoadedData& other);

    //! Recursively loads all supported config files from the given directory.
    //! Only handles files with known extensions.
    //!
    //! \param dir The config tree to load.
    //! \return False if the directory didn't exist or couldn't be read.
    bool loadConfigTree(const QString& dir);

    //! Convenience method, currently works with absolute paths only.
    //! See AVConfig2ImporterBase::import().
    bool load(const QString& file);

    //! The number of all elements. Also see getElementByTotalIndex().
    uint getElementTotalCount() const;
    bool isEmpty() const;
    //! The element with a specific index, using the total ordering. Also see getElementTotalCount().
    const LoadedElement& getElementByTotalIndex(uint index) const;

    template<typename T>
    uint getElementCount() const;
    //! *Attention* Must only be called if unique_elements is true!
    template<typename T>
    QStringList getElementKeys() const;

    template<typename T>
    T& getElementByIndex(uint index);
    template<typename T>
    const T& getElementByIndex(uint index) const;

    //! *Attention* Must only be called if unique_elements is true!
    template<typename T>
    T* getElementByName(const QString& name);
    //! *Attention* Must only be called if unique_elements is true!
    template<typename T>
    const T* getElementByName(const QString& name) const;

    /**
     * If an element already exists, it is modified but the order is not changed.
     * \return A pointer to the added / updated element.
     */
    template<typename T>
    T* addElement(const T& element);
    /**
     * *Attention* Must only be called if unique_elements is true!
     *
     * \return True if the element was removed, false if it did not exist.
     */
    template<typename T>
    bool removeElementByName(QString name);

    template<typename T>
    void removeElementByIndex(uint index);

    /**
     * Moves the given element to the back of the sorted lists (per-element and total).
     *
     * \param loaded_element Must be a valid pointer to an element in this object (e.g. returned by addElement).
     *                       Is invalidated by calling this method.
     * \return The new address of the element.
     */
    template<typename T>
    T* moveToBack(T* loaded_element);

    const std::set<AVConfig2StorageLocation>& getLoadedLocations() const;

    //! Convenience method when only one instance name is needed for SubconfigTemplateSpecification.
    //! Check the overloaded method for detailed documentation
    bool instantiateSubconfigTemplate(const SubconfigTemplateSpecification& spec, const QString& instance_name);

    /**
     * @brief instantiateSubconfigTemplate creates parameters for the given subconfig template.
     *
     * Creates parameters for the given subconfig template. Parameters will be stored at the same location as the
     * subconfig template. It does not create parameters for nested subconfig templates. For example, if we are
     * instantiating "alertchecker.filters.*.regions", only parameters that belong to the
     * "alertchecker.filters.*.regions" will be created. On the other hand, if we are instantiating
     * "alertchecker.filters", then only parameters that belong to the "alertchecker.filters" will be created.
     *
     * @param spec              Name and location of subconfig template. Name can have instance name for the nested
     *                          subconfig template already defined (e.g., "alertchecker.filters.filter_a.regions"
     *                          instead of "alertchecker.filters.*.regions")
     * @param instance_names    The names that should be used in place of unspecified namespaces ("*"). For example, if
     *                          subconfig template name is "alertchecker.filters.*.regions", two instance names must
     *                          be provided. If they are "filter_a" and "area1", then parameters would be created with a
     *                          prefix "alertchecker.filters.filter_a.regions.area1".
     * @return                  Whether instantiation was successful.
     *                          Instantiation can fail when there are not enough instance names provided or the
     *                          subconfig template specified in the spec does not exist.
     *                          In case of a failure, an error message is logged.
     */
    bool instantiateSubconfigTemplate(const SubconfigTemplateSpecification& spec, const QVector<QString>& instance_names);

    /**
     * @brief listSubconfigTemplates returns the list of subconfig templates that match the given prefix.
     *
     * @param prefix Parameter prefix that needs to match a subconfig template. For example:
     *               "alertchecker.filters.filter_a.regions" would match a subconfig template
     *               "alertchecker.filters.*.regions"
     */
    QList<SubconfigTemplateSpecification> listSubconfigTemplates(const QString& prefix) const;

    //! Implements the AVConfig2ImporterClient interface
    void addParameter(const QString& value, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addReference(const QString& ref, const AVConfig2Metadata& metadata,
                      const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addSearchPath(const QString& path, const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addInclude(const QString& include, const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addConfigMapping(const QString& map_from, const QString& mapped_dir, const QString& mapped_name,
                          const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addOverride(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addDefine(const QString& name, const QString& value, const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addInheritedSection(const QString& parent, const QString& section,
                             const AVConfig2StorageLocation& location) override;
    //! Implements the AVConfig2ImporterClient interface
    void addSubconfigTemplateParameter(const QString& value, const AVConfig2Metadata& metadata,
                                       const AVConfig2StorageLocation& location) override;

private:
    template<typename T>
    QList<T>& getElementContainer();
    template<typename T>
    const QList<T>& getElementContainer() const;

    bool m_unique_elements;

    QList<LoadedParameter>                  m_loaded_parameter;
    QList<LoadedReference>                  m_loaded_reference;
    QList<LoadedMapping>                    m_loaded_mapping;
    QList<LoadedSearchPath>                 m_loaded_search_path;
    QList<LoadedInclude>                    m_loaded_include;
    QList<LoadedOverride>                   m_loaded_override;
    QList<LoadedDefine>                     m_loaded_define;
    QList<LoadedInheritance>                m_loaded_inheritance;
    QList<LoadedSubconfigTemplateParameter> m_loaded_subconfig_template_parameter;

    typedef QMap<QString, LoadedElement*> ElementDict;
    //! Maps container pointer (m_loaded_XXX members above) to type specific element dict
    typedef QMap<const void*, ElementDict> ElementTypeToDictContainer;
    //! *Attention* Only ever used if unique_elements is true!
    ElementTypeToDictContainer m_loaded_element_dicts;

    typedef QList<LoadedElement*> TotalElementContainer;
    TotalElementContainer m_total_element_list;

    //! Remember all locations which were ever used while loading data; this is used to save those
    //! locations back out again even if there are no more elements.
    std::set<AVConfig2StorageLocation> m_loaded_locations;
};

///////////////////////////////////////////////////////////////////////////////
// template specialization need to be declared in header for Windows compatibility (DLL)

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedParameter>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedReference>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedMapping>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedSearchPath>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedInclude>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedOverride>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedDefine>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedInheritance>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<>
AVLIB_EXPORT QList<AVConfig2ImporterClient::LoadedSubconfigTemplateParameter>& AVConfig2LoadedData::getElementContainer();

///////////////////////////////////////////////////////////////////////////////

template<typename T>
uint AVConfig2LoadedData::getElementCount() const
{
    return getElementContainer<T>().count();
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
QStringList AVConfig2LoadedData::getElementKeys() const
{
    AVASSERT(m_unique_elements);

    ElementTypeToDictContainer::const_iterator it = m_loaded_element_dicts.find(&getElementContainer<T>());
    AVASSERT(it != m_loaded_element_dicts.end()); // ensured at construction time

    ElementDict dict = it.value();
    return dict.keys();
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
const T& AVConfig2LoadedData::getElementByIndex(uint index) const
{
    AVASSERT(static_cast<int>(index) < getElementContainer<T>().count());
    return getElementContainer<T>()[index];
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
T& AVConfig2LoadedData::getElementByIndex(uint index)
{
    AVASSERT(static_cast<int>(index) < getElementContainer<T>().count());
    return getElementContainer<T>()[index];
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
T* AVConfig2LoadedData::getElementByName(const QString& name)
{
    AVASSERT(m_unique_elements);

    ElementDict& dict = m_loaded_element_dicts[&getElementContainer<T>()];
    ElementDict::iterator it = dict.find(name);
    if (it == dict.end()) return nullptr;
    T* ret = dynamic_cast<T*>(*it);
    AVASSERT(ret != nullptr);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
const T* AVConfig2LoadedData::getElementByName(const QString& name) const
{
    AVASSERT(m_unique_elements);

    const ElementDict& dict = m_loaded_element_dicts[&getElementContainer<T>()];
    ElementDict::const_iterator it = dict.find(name);
    if (it == dict.end()) return nullptr;
    const T* ret = dynamic_cast<const T*>(*it);
    AVASSERT(ret != nullptr);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
T *AVConfig2LoadedData::addElement(const T& element)
{
    T* ret = nullptr;

    if (m_unique_elements)
    {
        T* existing_element = getElementByName<T>(element.getName());
        if (existing_element != nullptr)
        {
            *existing_element = element;
            ret = existing_element;
        }
    }

    if (ret == nullptr)
    {
        typedef QList<T> ContainerType;
        ContainerType * container = &getElementContainer<T>();
        container->push_back(element);
        ret = &container->back();

        m_total_element_list.push_back(ret);
        if (m_unique_elements)
        {
            m_loaded_element_dicts[container][element.getName()] = ret;
        }
    }

    // location might change for existing element, so always do this.
    m_loaded_locations.insert(ret->getLocation());
//    AVASSERT(ret->getLocation().m_source != AVConfig2StorageLocation::PS_UNKNOWN);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool AVConfig2LoadedData::removeElementByName(QString name)
{
    AVASSERT(m_unique_elements);

    T * element_to_remove = getElementByName<T>(name);
    if (element_to_remove == nullptr) return false;

    typedef QList<T> ContainerType;
    ContainerType& container = getElementContainer<T>();

    AVASSERT(container.removeAll(*element_to_remove) == 1);

    AVASSERT(m_total_element_list.removeAll(element_to_remove) == 1);
    if (m_unique_elements)
    {
        m_loaded_element_dicts[&container].remove(name);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
void AVConfig2LoadedData::removeElementByIndex(uint index)
{
    T& element_to_remove = getElementByIndex<T>(index);

    typedef QList<T> ContainerType;
    ContainerType& container = getElementContainer<T>();

    if (m_unique_elements)
    {
        m_loaded_element_dicts[&container].remove(element_to_remove.getName());
    }
    AVASSERT(m_total_element_list.removeAll(&element_to_remove) == 1);
    AVASSERT(container.removeAll(element_to_remove) == 1);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
T* AVConfig2LoadedData::moveToBack(T* loaded_element)
{
    AVASSERT(m_total_element_list.removeAll(dynamic_cast<LoadedElement*>(loaded_element)) == 1);

    // Modifying the list below changes the address of the element...
    typedef QList<T> ContainerType;
    ContainerType& container = getElementContainer<T>();
    T copy = *loaded_element;
    AVASSERT(container.removeAll(*loaded_element) == 1);
    container.push_back(copy);
    loaded_element = &container.back();

    m_total_element_list.push_back(loaded_element);

    if (m_unique_elements)
    {
        m_loaded_element_dicts[&container][loaded_element->getName()] = loaded_element;
    }

    return loaded_element;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
const QList<T>& AVConfig2LoadedData::getElementContainer() const
{
    // Ugly const cast. The alternative is to do all specializations for the const variant as well...
    return const_cast<AVConfig2LoadedData*>(this)->getElementContainer<T>();
}

#endif // AVCONFIG2LOADEDDATA_INCLUDED

// End of file
