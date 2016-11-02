#pragma once

#include <ToolsFoundation/Basics.h>
#include <Foundation/Strings/String.h>
#include <ToolsFoundation/Basics/Status.h>
#include <ToolsFoundation/Selection/SelectionManager.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Logging/Log.h>
#include <ToolsFoundation/Document/Implementation/Declarations.h>
#include <CoreUtils/DataStructures/ObjectMetaData.h>
#include <Foundation/Types/UniquePtr.h>

class ezObjectAccessorBase;
class ezObjectCommandAccessor;

struct EZ_TOOLSFOUNDATION_DLL ezObjectAccessorChangeEvent
{
  ezDocument* m_pDocument;
  ezObjectAccessorBase* m_pOldObjectAccessor;
  ezObjectAccessorBase* m_pNewObjectAccessor;
};

class EZ_TOOLSFOUNDATION_DLL ezDocumentObjectMetaData : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezDocumentObjectMetaData, ezReflectedClass);

public:

  enum ModifiedFlags
  {
    HiddenFlag = EZ_BIT(0),
    PrefabFlag = EZ_BIT(1),

    AllFlags = 0xFFFFFFFF
  };

  ezDocumentObjectMetaData()
  {
    m_bHidden = false;
  }

  bool m_bHidden; /// Whether the object should be rendered in the editor view (no effect on the runtime)
  ezUuid m_CreateFromPrefab; /// The asset GUID of the prefab from which this object was created. Invalid GUID, if this is not a prefab instance.
  ezUuid m_PrefabSeedGuid; /// The seed GUID used to remap the object GUIDs from the prefab asset into this instance.
  ezString m_sBasePrefab; /// The prefab from which this instance was created as complete JSON text (this describes the entire object!). Necessary for three-way-merging the prefab instances.
};

class EZ_TOOLSFOUNDATION_DLL ezDocument : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezDocument, ezReflectedClass);

public:
  ezDocument(const char* szPath, ezDocumentObjectManager* pDocumentObjectManagerImpl);
  virtual ~ezDocument();

  /// \name Document State Functions
  ///@{

  bool IsModified() const { return m_bModified; }
  bool IsReadOnly() const { return m_bReadOnly; }
  const ezUuid& GetGuid() const { return m_pDocumentInfo->m_DocumentID; }
  virtual const char* GetDocumentTypeDisplayString() const = 0;

  const ezDocumentObjectManager* GetObjectManager() const { return m_pObjectManager; }
  ezDocumentObjectManager* GetObjectManager() { return m_pObjectManager; }
  ezSelectionManager* GetSelectionManager() const { return &m_SelectionManager; }
  ezCommandHistory* GetCommandHistory() const { return &m_CommandHistory; }
  virtual ezObjectAccessorBase* GetObjectAccessor() const;

  virtual ezVariant GetDefaultValue(const ezDocumentObject* pObject, const char* szProperty, ezVariant index = ezVariant()) const;
  virtual bool IsDefaultValue(const ezDocumentObject* pObject, const char* szProperty, bool bReturnOnInvalid, ezVariant index = ezVariant()) const;

  ///@}
  /// \name Document Management Functions
  ///@{

  /// \brief Returns the absolute path to the document.
  const char* GetDocumentPath() const { return m_sDocumentPath; }

  ezStatus SaveDocument();
  ezStatus LoadDocument() { return InternalLoadDocument(); }

  /// \brief Brings the corresponding window to the front.
  void EnsureVisible();

  ezDocumentManager* GetDocumentManager() const { return m_pDocumentManager; }

  bool HasWindowBeenRequested() const { return m_bWindowRequested; }

  const ezDocumentTypeDescriptor* GetDocumentTypeDescriptor() const { return m_pTypeDescriptor; }

  const ezDocumentInfo* GetDocumentInfo() { return m_pDocumentInfo; }

  ///@}
  /// \name Clipboard Functions
  ///@{

  struct PasteInfo
  {
    EZ_DECLARE_POD_TYPE();

    ezDocumentObject* m_pObject;
    ezDocumentObject* m_pParent;
  };

  virtual bool Copy(ezAbstractObjectGraph& out_objectGraph) const { return false; };
  virtual bool Paste(const ezArrayPtr<PasteInfo>& info, const ezAbstractObjectGraph& objectGraph, bool bAllowPickedPosition) { return false; };

  ///@}
  /// \name Misc Functions
  ///@{

  virtual void DeleteSelectedObjects() const;

  const ezSet<ezString>& GetUnknownObjectTypes() const { return m_UnknownObjectTypes; }
  ezUInt32 GetUnknownObjectTypeInstances() const { return m_uiUnknownObjectTypeInstances; }

  /// \brief If disabled, this document will not be put into the recent files list.
  void SetAddToResetFilesList(bool b) { m_bAddToRecentFilesList = b; }

  /// \brief Whether this document shall be put into the recent files list.
  bool GetAddToRecentFilesList() const { return m_bAddToRecentFilesList; }

  /// \brief Broadcasts a status message event. The window that displays the document may show this in some form, e.g. in the status bar.
  void ShowDocumentStatus(const char* szFormat, ...) const;

  /// \brief Tries to compute the position and rotation for an object in the document. Returns EZ_SUCCESS if it was possible.
  virtual ezResult ComputeObjectTransformation(const ezDocumentObject* pObject, ezTransform& out_Result) const { return EZ_FAILURE; }

  ///@}
  /// \name Prefab Functions
  ///@{

  /// \brief Whether the document allows to create prefabs in it. This may note be allowed for prefab documents themselves, to prevent nested prefabs.
  virtual bool ArePrefabsAllowed() const { return true; }

  /// \brief Updates ALL prefabs in the document with the latest changes. Merges the current prefab templates with the instances in the document.
  virtual void UpdatePrefabs();

  /// \brief Resets the given objects to their template prefab state, if they have local modifications.
  void RevertPrefabs(const ezDeque<const ezDocumentObject*>& Selection);

  /// \brief Removes the link between a prefab instance and its template, turning the instance into a regular object.
  virtual void UnlinkPrefabs(const ezDeque<const ezDocumentObject*>& Selection);

  virtual ezStatus CreatePrefabDocumentFromSelection(const char* szFile, const ezRTTI* pRootType);
  virtual ezStatus CreatePrefabDocument(const char* szFile, const ezDocumentObject* pRootObject, const ezUuid& invPrefabSeed, ezUuid& out_NewDocumentGuid);
  // Returns new guid of replaced object.
  virtual ezUuid ReplaceByPrefab(const ezDocumentObject* pRootObject, const char* szPrefabFile, const ezUuid& PrefabAsset, const ezUuid& PrefabSeed);
  // Returns new guid of reverted object.
  virtual ezUuid RevertPrefab(const ezDocumentObject* pObject);

  ///@}

public:
  ezObjectMetaData<ezUuid, ezDocumentObjectMetaData> m_DocumentObjectMetaData;
  
  mutable ezEvent<const ezDocumentEvent&> m_EventsOne;
  static ezEvent<const ezDocumentEvent&> s_EventsAny;

  mutable ezEvent<const ezObjectAccessorChangeEvent&> m_ObjectAccessorChangeEvents;

protected:
  void SetModified(bool b);
  void SetReadOnly(bool b);
  virtual ezStatus InternalSaveDocument();
  virtual ezStatus InternalLoadDocument();
  virtual ezDocumentInfo* CreateDocumentInfo() = 0;

  /// \brief A hook to execute additional code after SUCCESSFULLY saving a document. E.g. manual asset transform can be done here.
  virtual void InternalAfterSaveDocument() {}

  virtual void AttachMetaDataBeforeSaving(ezAbstractObjectGraph& graph) const;
  virtual void RestoreMetaDataAfterLoading(const ezAbstractObjectGraph& graph);

  virtual void InitializeBeforeLoading() {}
  virtual void InitializeAfterLoading() {}

  void SetUnknownObjectTypes(const ezSet<ezString>& Types, ezUInt32 uiInstances);

  /// \name Prefab Functions
  ///@{

  virtual void UpdatePrefabsRecursive(ezDocumentObject* pObject);
  virtual void UpdatePrefabObject(ezDocumentObject* pObject, const ezUuid& PrefabAsset, const ezUuid& PrefabSeed, const char* szBasePrefab);
  virtual ezString GetDocumentPathFromGuid(const ezUuid& documentGuid) const;

  ///@}

  mutable ezSelectionManager m_SelectionManager;
  mutable ezCommandHistory m_CommandHistory;
  ezDocumentInfo* m_pDocumentInfo;
  const ezDocumentTypeDescriptor* m_pTypeDescriptor;
  mutable ezObjectCommandAccessor* m_ObjectAccessor; ///< Default object accessor used by every doc.

private:
  friend class ezDocumentManager;
  friend class ezCommandHistory;

  void SetupDocumentInfo(const ezDocumentTypeDescriptor* pTypeDescriptor);

  ezDocumentManager* m_pDocumentManager;
  ezDocumentObjectManager* m_pObjectManager;

  ezString m_sDocumentPath;
  bool m_bModified;
  bool m_bReadOnly;
  bool m_bWindowRequested;
  bool m_bAddToRecentFilesList;

  ezSet<ezString> m_UnknownObjectTypes;
  ezUInt32 m_uiUnknownObjectTypeInstances;
};
