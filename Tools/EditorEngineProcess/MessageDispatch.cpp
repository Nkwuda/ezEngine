#include <PCH.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <EditorFramework/IPC/ProcessCommunication.h>
#include <EditorFramework/EngineProcess/EngineProcessMessages.h>
#include <EditorFramework/EngineProcess/EngineProcessViewContext.h>
#include <EditorFramework/EngineProcess/EngineProcessDocumentContext.h>
#include <EditorEngineProcess/GameState.h>
#include <Foundation/Reflection/ReflectionSerializer.h>

#include <RendererCore/Meshes/MeshComponent.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <GameUtils/Components/RotorComponent.h>
#include <GameUtils/Components/SliderComponent.h>

void ezEngineProcessGameState::SendProjectReadyMessage()
{
  ezProjectReadyMsgToEditor msg;
  m_IPC.SendMessage(&msg);
}


void ezEngineProcessGameState::SendReflectionInformation()
{
  ezSet<const ezRTTI*> types;
  ezReflectionUtils::GatherTypesDerivedFromClass(ezGetStaticRTTI<ezComponent>(), types, true);
  ezDynamicArray<const ezRTTI*> sortedTypes;
  ezReflectionUtils::CreateDependencySortedTypeArray(types, sortedTypes);

  for (auto type : sortedTypes)
  {
    ezUpdateReflectionTypeMsgToEditor TypeMsg;
    ezToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(type, TypeMsg.m_desc);
    m_IPC.SendMessage(&TypeMsg);
  }
}

void ezEngineProcessGameState::EventHandlerIPC(const ezProcessCommunication::Event& e)
{
  // Project Messages:
  if (e.m_pMessage->GetDynamicRTTI()->IsDerivedFrom<ezSetupProjectMsgToEditor>())
  {
    const ezSetupProjectMsgToEditor* pSetupMsg = static_cast<const ezSetupProjectMsgToEditor*>(e.m_pMessage);
    ezApplicationConfig::SetProjectDirectory(pSetupMsg->m_sProjectDir);

    const_cast<ezSetupProjectMsgToEditor*>(pSetupMsg)->m_Config.Apply();
    // Project setup, we are now ready to accept document messages.
    SendProjectReadyMessage();
    return;
  }

  // Document Messages:
  if (!e.m_pMessage->GetDynamicRTTI()->IsDerivedFrom<ezEditorEngineDocumentMsg>())
    return;
  const ezEditorEngineDocumentMsg* pDocMsg = (const ezEditorEngineDocumentMsg*) e.m_pMessage;

  ezViewContext* pViewContext = (ezViewContext*) ezEngineProcessViewContext::GetViewContext(pDocMsg->m_uiViewID);

  if (pViewContext == nullptr && pDocMsg->m_uiViewID != 0xFFFFFFFF)
  {
    ezLog::Info("Created new View 0x%08X for document %s", pDocMsg->m_uiViewID, ezConversionUtils::ToString(pDocMsg->m_DocumentGuid).GetData());

    pViewContext = EZ_DEFAULT_NEW(ezViewContext, pDocMsg->m_uiViewID, pDocMsg->m_DocumentGuid);

    ezEngineProcessViewContext::AddViewContext(pDocMsg->m_uiViewID, pViewContext);

    EZ_ASSERT_DEV(pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezDocumentOpenMsgToEngine>(), "The first message from a new view has to be of type ezDocumentOpenMsgToEngine. This message is of type '%s'", pDocMsg->GetDynamicRTTI()->GetTypeName());


  }

  ezEngineProcessDocumentContext* pDocumentContext = ezEngineProcessDocumentContext::GetDocumentContext(pDocMsg->m_DocumentGuid);
  
  if (pDocumentContext == nullptr && pDocMsg->m_DocumentGuid.IsValid())
  {
    ezLog::Info("Created new Document context for Guid %s", ezConversionUtils::ToString(pDocMsg->m_DocumentGuid).GetData());

    pDocumentContext = EZ_DEFAULT_NEW(ezEngineProcessDocumentContext);

    ezEngineProcessDocumentContext::AddDocumentContext(pDocMsg->m_DocumentGuid, pDocumentContext);

    pDocumentContext->m_pWorld = EZ_DEFAULT_NEW(ezWorld, ezConversionUtils::ToString(pDocMsg->m_DocumentGuid));
    EZ_LOCK(pDocumentContext->m_pWorld->GetWriteMarker());

    pDocumentContext->m_pWorld->CreateComponentManager<ezMeshComponentManager>();
    pDocumentContext->m_pWorld->CreateComponentManager<ezRotorComponentManager>();
    pDocumentContext->m_pWorld->CreateComponentManager<ezSliderComponentManager>();
  }

  EZ_LOCK(pDocumentContext->m_pWorld->GetWriteMarker());


  if (pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezDocumentOpenMsgToEngine>()) // Document was opened or closed
  {
    ezDocumentOpenMsgToEngine* pMsg = (ezDocumentOpenMsgToEngine*) pDocMsg;

    if (pMsg->m_bDocumentOpen)
    {
      ezDocumentOpenResponseMsgToEditor m;
      m.m_uiViewID = pMsg->m_uiViewID;
      m.m_DocumentGuid = pMsg->m_DocumentGuid;

      m_IPC.SendMessage(&m);
    }
    else
    {
      ezEngineProcessDocumentContext::DestroyDocumentContext(pDocMsg->m_DocumentGuid);
    }
  }
  else if (pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezViewRedrawMsgToEngine>())
  {
    ezViewRedrawMsgToEngine* pMsg = (ezViewRedrawMsgToEngine*) pDocMsg;

    pViewContext->SetupRenderTarget(reinterpret_cast<HWND>(pMsg->m_uiHWND), pMsg->m_uiWindowWidth, pMsg->m_uiWindowHeight);
    pViewContext->Redraw();
  }
  else if (pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezEntityMsgToEngine>())
  {
    ezEntityMsgToEngine* pMsg = (ezEntityMsgToEngine*) pDocMsg;

    HandlerEntityMsg(pDocumentContext, pViewContext, pMsg);

  }
  else if (pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezViewCameraMsgToEngine>())
  {
    ezViewCameraMsgToEngine* pMsg = (ezViewCameraMsgToEngine*) pDocMsg;

    pViewContext->SetCamera(pMsg);
  }
  else if (pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezEditorEngineSyncObjectMsg>())
  {
    ezEditorEngineSyncObjectMsg* pMsg = (ezEditorEngineSyncObjectMsg*) pDocMsg;

    pDocumentContext->ProcessEditorEngineSyncObjectMsg(*pMsg);
  }
  else if (pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezViewPickingMsgToEngine>())
  {
    ezViewPickingMsgToEngine* pMsg = (ezViewPickingMsgToEngine*)pDocMsg;

    pViewContext->PickObjectAt(pMsg->m_uiPickPosX, pMsg->m_uiPickPosY);
  }
  else if (pDocMsg->GetDynamicRTTI()->IsDerivedFrom<ezViewHighlightMsgToEngine>())
  {
    ezViewHighlightMsgToEngine* pMsg = (ezViewHighlightMsgToEngine*)pDocMsg;

    ezUInt32 uiPickingID = m_OtherPickingMap.GetHandle(pMsg->m_HighlightObject);
    
    ezRenderContext::GetDefaultInstance()->SetMaterialParameter("PickingHighlightID", (ezInt32) uiPickingID);

    //ezLog::Info("Picking: GUID = %s, ID = %u", ezConversionUtils::ToString(pMsg->m_HighlightObject).GetData(), uiPickingID);
  }
}

void ezEngineProcessGameState::UpdateProperties(ezEntityMsgToEngine* pMsg, void* pObject, const ezRTTI* pRtti)
{
  ezMemoryStreamStorage storage;
  ezMemoryStreamWriter writer(&storage);
  ezMemoryStreamReader reader(&storage);

  writer.WriteBytes(pMsg->m_sObjectData.GetData(), pMsg->m_sObjectData.GetElementCount());

  ezReflectionSerializer::ReadObjectPropertiesFromJSON(reader, *pRtti, pObject);
}

void ezEngineProcessGameState::HandlerGameObjectMsg(ezEngineProcessDocumentContext* pDocumentContext, ezViewContext* pViewContext, ezEntityMsgToEngine* pMsg, ezRTTI* pRtti)
{
  switch (pMsg->m_iMsgType)
  {
  case ezEntityMsgToEngine::ObjectAdded:
    {
      ezGameObjectDesc d;
      d.m_sName.Assign(ezConversionUtils::ToString(pMsg->m_ObjectGuid).GetData());

      if (pMsg->m_NewParentGuid.IsValid())
        d.m_Parent = m_GameObjectMap.GetHandle(pMsg->m_NewParentGuid);

      ezGameObjectHandle hObject = pDocumentContext->m_pWorld->CreateObject(d);
      m_GameObjectMap.RegisterObject(pMsg->m_ObjectGuid, hObject);

      ezGameObject* pObject;
      if (pDocumentContext->m_pWorld->TryGetObject(hObject, pObject))
      {
        UpdateProperties(pMsg, pObject, ezGetStaticRTTI<ezGameObject>());
      }
    }
    break;

  case ezEntityMsgToEngine::ObjectMoved:
    {
      ezRTTI* pRtti = ezRTTI::FindTypeByName(pMsg->m_sObjectType);

      if (pRtti == nullptr)
      {
        ezLog::Error("Cannot create object of type '%s', RTTI is unknown", pMsg->m_sObjectType.GetData());
        break;
      }

      ezGameObjectHandle hObject = m_GameObjectMap.GetHandle(pMsg->m_ObjectGuid);

      ezGameObjectHandle hNewParent;
      if (pMsg->m_NewParentGuid.IsValid())
        hNewParent = m_GameObjectMap.GetHandle(pMsg->m_NewParentGuid);

      ezGameObject* pObject = nullptr;
      if (pDocumentContext->m_pWorld->TryGetObject(hObject, pObject))
      {
        pObject->SetParent(hNewParent);
      }
      else
        ezLog::Error("Couldn't access game object object %s in world %p", ezConversionUtils::ToString(pMsg->m_ObjectGuid).GetData(), pDocumentContext->m_pWorld);
    }
    break;

  case ezEntityMsgToEngine::ObjectRemoved:
    {
      pDocumentContext->m_pWorld->DeleteObject(m_GameObjectMap.GetHandle(pMsg->m_ObjectGuid));
      m_GameObjectMap.UnregisterObject(pMsg->m_ObjectGuid);
    }
    break;

  case ezEntityMsgToEngine::PropertyChanged:
    {
      ezGameObjectHandle hObject = m_GameObjectMap.GetHandle(pMsg->m_ObjectGuid);

      ezGameObject* pObject;
      if (pDocumentContext->m_pWorld->TryGetObject(hObject, pObject))
      {
        UpdateProperties(pMsg, pObject, ezGetStaticRTTI<ezGameObject>());
      }
    }
    break;
  }
}

void ezEngineProcessGameState::HandleComponentMsg(ezEngineProcessDocumentContext* pDocumentContext, ezViewContext* pViewContext, ezEntityMsgToEngine* pMsg, ezRTTI* pRtti)
{
  ezComponentManagerBase* pMan = pDocumentContext->m_pWorld->GetComponentManager(pRtti);

  if (pMan == nullptr)
  {
    ezLog::Error("Component of type '%s' cannot be created, no component manager is registered", pRtti->GetTypeName());
    return;
  }

  switch (pMsg->m_iMsgType)
  {
  case ezEntityMsgToEngine::ObjectAdded:
    {
      ezComponentHandle hComponent = pMan->CreateComponent();

      ezGameObjectHandle hParent = m_GameObjectMap.GetHandle(pMsg->m_NewParentGuid);
      ezGameObject* pParent;
      if (!pDocumentContext->m_pWorld->TryGetObject(hParent, pParent))
        break;

      ezComponent* pComponent;
      if (pMan->TryGetComponent(hComponent, pComponent))
      {
        m_ComponentMap.RegisterObject(pMsg->m_ObjectGuid, hComponent);
        UpdateProperties(pMsg, pComponent, pComponent->GetDynamicRTTI());

        pComponent->m_uiEditorPickingID = m_uiNextComponentPickingID++;

        m_ComponentPickingMap.RegisterObject(pMsg->m_ObjectGuid, pComponent->m_uiEditorPickingID);
      }
      else
      {
        ezLog::Error("Component of type '%s' cannot be found after creation", pRtti->GetTypeName());
      }

      pParent->AddComponent(hComponent);
    }
    break;

  case ezEntityMsgToEngine::ObjectMoved:
    {
      ezGameObjectHandle hParent = m_GameObjectMap.GetHandle(pMsg->m_NewParentGuid);
      ezGameObject* pParent;
      if (!pDocumentContext->m_pWorld->TryGetObject(hParent, pParent))
        break;

      ezComponentHandle hComponent = m_ComponentMap.GetHandle(pMsg->m_ObjectGuid);

      ezComponent* pComponent;
      if (pMan->TryGetComponent(hComponent, pComponent))
      {
        if (pComponent->GetOwner())
          pComponent->GetOwner()->RemoveComponent(pComponent);
      }

      pParent->AddComponent(hComponent);
    }
    break;

  case ezEntityMsgToEngine::ObjectRemoved:
    {
      ezComponentHandle hComponent = m_ComponentMap.GetHandle(pMsg->m_ObjectGuid);
      m_ComponentMap.UnregisterObject(pMsg->m_ObjectGuid);
      m_ComponentPickingMap.UnregisterObject(pMsg->m_ObjectGuid);

      pMan->DeleteComponent(hComponent);
    }
    break;

  case ezEntityMsgToEngine::PropertyChanged:
    {
      ezComponentHandle hComponent = m_ComponentMap.GetHandle(pMsg->m_ObjectGuid);

      ezComponent* pComponent;
      if (pMan->TryGetComponent(hComponent, pComponent))
      {
        UpdateProperties(pMsg, pComponent, pComponent->GetDynamicRTTI());
      }
      else
      {
        ezLog::Error("Component of type '%s' cannot be found", pRtti->GetTypeName());
      }
    }
    break;
  }

}

void ezEngineProcessGameState::HandlerEntityMsg(ezEngineProcessDocumentContext* pDocumentContext, ezViewContext* pViewContext, ezEntityMsgToEngine* pMsg)
{

  ezRTTI* pRtti = ezRTTI::FindTypeByName(pMsg->m_sObjectType);

  if (pRtti == nullptr)
  {
    ezLog::Error("Cannot create object of type '%s', RTTI is unknown", pMsg->m_sObjectType.GetData());
    return;
  }

  if (pRtti == ezGetStaticRTTI<ezGameObject>())
  {
    HandlerGameObjectMsg(pDocumentContext, pViewContext, pMsg, pRtti);
  }

  if (pRtti->IsDerivedFrom<ezComponent>())
  {
    HandleComponentMsg(pDocumentContext, pViewContext, pMsg, pRtti);
  }

}




