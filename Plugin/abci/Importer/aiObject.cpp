#include "pch.h"
#include "aiInternal.h"
#include "aiContext.h"
#include "aiObject.h"
#include "aiSchema.h"
#include "aiXForm.h"
#include "aiPolyMesh.h"
#include "aiCamera.h"
#include "aiPoints.h"

aiObject::aiObject()
{
}

aiObject::aiObject(aiContext *ctx, aiObject *parent, const abcObject &abc)
    : m_ctx(ctx)
    , m_abc(abc)
    , m_parent(parent)
{
    if (m_abc.valid())
    {
        const auto& metadata = m_abc.getMetaData();
        
        if (AbcGeom::IXformSchema::matches(metadata))
        {
            m_xform.reset(new aiXForm(this));
            m_schemas.push_back(m_xform.get());
        }
        
        if (AbcGeom::IPolyMeshSchema::matches(metadata))
        {
            m_polymesh.reset(new aiPolyMesh(this));
            m_schemas.push_back(m_polymesh.get());
        }

        if (AbcGeom::ICameraSchema::matches(metadata))
        {
            m_camera.reset(new aiCamera(this));
            m_schemas.push_back(m_camera.get());
        }

        if (AbcGeom::IPointsSchema::matches(metadata))
        {
            m_points.reset(new aiPoints(this));
            m_schemas.push_back(m_points.get());
        }
    }
}

aiObject::~aiObject()
{
    while (!m_children.empty()) {
        delete m_children.back();
    }
    if (m_parent != nullptr) {
        m_parent->removeChild(this);
    }
}

aiObject* aiObject::newChild(const abcObject &abc)
{
    auto *child = new aiObject(getContext(), this, abc);
    m_children.push_back(child);
    return child;
}

void aiObject::removeChild(aiObject *c)
{
    if (c == nullptr) { return; }

    auto it = std::find(m_children.begin(), m_children.end(), c);
    if (it != m_children.end())
    {
        c->m_parent = nullptr;
        m_children.erase(it);
    }
}

void aiObject::readConfig()
{
    for (auto s : m_schemas)
    {
        s->readConfig();
    }
}

void aiObject::updateSample(const abcSampleSelector& ss)
{
    DebugLog("aiObject::updateSample(obj='%s', t=%f)", getFullName(), ss.getRequestedTime());
    for (auto s : m_schemas)
    {
        s->updateSample(ss);
    }
}

aiContext*  aiObject::getContext() const { return m_ctx; }
abcObject&  aiObject::getAbcObject() { return m_abc; }
const char* aiObject::getName() const { return m_abc.getName().c_str(); }
const char* aiObject::getFullName() const { return m_abc.getFullName().c_str(); }
uint32_t    aiObject::getNumChildren() const { return (uint32_t)m_children.size(); }
aiObject*   aiObject::getChild(int i) { return m_children[i]; }
aiObject*   aiObject::getParent() const { return m_parent; }

void aiObject::cacheSamples(int64_t startIndex, int64_t endIndex)
{
    for (auto s : m_schemas)
    {
        s->cacheSamples(startIndex,endIndex);
    }
}

aiXForm*    aiObject::getXForm() const { return m_xform.get(); }
aiPolyMesh* aiObject::getPolyMesh() const { return m_polymesh.get(); }
aiCamera*   aiObject::getCamera() const { return m_camera.get(); }
aiPoints*   aiObject::getPoints() const { return m_points.get(); }


