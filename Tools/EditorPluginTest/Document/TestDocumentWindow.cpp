#include <PCH.h>
#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <EditorPluginTest/Document/TestDocumentWindow.moc.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <CoreUtils/Geometry/GeomUtils.h>
#include <CoreUtils/Geometry/OBJLoader.h>
#include <Foundation/IO/OSFile.h>
#include <QTimer>
#include <QPushButton>
#include <EditorFramework/EngineProcess/EngineProcessMessages.h>
#include <qlayout.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Core/World/GameObject.h>
#include <QKeyEvent>
#include <Foundation/Time/Time.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/ToolBarActionMapView.moc.h>

ezEditorInputContext* ezEditorInputContext::s_pActiveInputContext = nullptr;

ezTestDocumentWindow::ezTestDocumentWindow(ezDocumentBase* pDocument) : ezDocumentWindow3D(pDocument)
{
  m_pCenterWidget = new ez3DViewWidget(this, this);
  m_pCenterWidget->m_MoveContext.SetCamera(&m_Camera);

  m_pCenterWidget->setAutoFillBackground(false);
  setCentralWidget(m_pCenterWidget);

  SetTargetFramerate(24);

  m_DelegatePropertyEvents = ezMakeDelegate(&ezTestDocumentWindow::PropertyEventHandler, this);
  m_DelegateDocumentTreeEvents = ezMakeDelegate(&ezTestDocumentWindow::DocumentTreeEventHandler, this);

  GetDocument()->GetObjectTree()->m_StructureEvents.AddEventHandler(m_DelegateDocumentTreeEvents);
  GetDocument()->GetObjectTree()->m_PropertyEvents.AddEventHandler(m_DelegatePropertyEvents);

  m_Camera.SetCameraMode(ezCamera::CameraMode::PerspectiveFixedFovY, 80.0f, 0.1f, 1000.0f);
  m_Camera.LookAt(ezVec3(0.5f, 1.5f, 2.0f), ezVec3(0.0f, 0.5f, 0.0f), ezVec3(0.0f, 1.0f, 0.0f));

  m_pCenterWidget->m_MoveContext.LoadState();

  {
    // Menu Bar
    ezMenuBarActionMapView* pMenuBar = static_cast<ezMenuBarActionMapView*>(menuBar());
    ezActionContext context;
    context.m_sMapping = "EditorTestDocumentMenuBar";
    context.m_pDocument = pDocument;
    pMenuBar->SetActionContext(context);
  }

  {
    // Tool Bar
    ezToolBarActionMapView* pToolBar = new ezToolBarActionMapView(this);
    ezActionContext context;
    context.m_sMapping = "EditorTestDocumentToolBar";
    context.m_pDocument = pDocument;
    pToolBar->SetActionContext(context);
    pToolBar->setObjectName("Test Document Window Tool Bar");
    addToolBar(pToolBar);
  }
}

ezTestDocumentWindow::~ezTestDocumentWindow()
{
  GetDocument()->GetObjectTree()->m_PropertyEvents.RemoveEventHandler(m_DelegatePropertyEvents);
  GetDocument()->GetObjectTree()->m_StructureEvents.RemoveEventHandler(m_DelegateDocumentTreeEvents);
}

void ezTestDocumentWindow::PropertyEventHandler(const ezDocumentObjectTreePropertyEvent& e)
{
  m_pEngineView->SendObjectProperties(e);
}

void ezTestDocumentWindow::DocumentTreeEventHandler(const ezDocumentObjectTreeStructureEvent& e)
{
  m_pEngineView->SendDocumentTreeChange(e);
}

void ezTestDocumentWindow::InternalRedraw()
{
  ezDocumentWindow3D::SyncObjects();

  ezEditorInputContext::UpdateActiveInputContext();

  SendRedrawMsg();
}

void ezTestDocumentWindow::SendRedrawMsg()
{
  ezViewCameraMsgToEngine cam;
  cam.m_fNearPlane = m_Camera.GetNearPlane();
  cam.m_fFarPlane = m_Camera.GetFarPlane();
  cam.m_iCameraMode = (ezInt8)m_Camera.GetCameraMode();
  cam.m_fFovOrDim = m_Camera.GetFovOrDim();
  cam.m_vDirForwards = m_Camera.GetCenterDirForwards();
  cam.m_vDirUp = m_Camera.GetCenterDirUp();
  cam.m_vDirRight = m_Camera.GetCenterDirRight();
  cam.m_vPosition = m_Camera.GetCenterPosition();
  m_Camera.GetViewMatrix(cam.m_ViewMatrix);
  m_Camera.GetProjectionMatrix((float)m_pCenterWidget->width() / (float)m_pCenterWidget->height(), cam.m_ProjMatrix);

  m_pEngineView->SendMessage(&cam);

  ezViewRedrawMsgToEngine msg;
  msg.m_uiHWND = (ezUInt64)(m_pCenterWidget->winId());
  msg.m_uiWindowWidth = m_pCenterWidget->width();
  msg.m_uiWindowHeight = m_pCenterWidget->height();

  m_pEngineView->SendMessage(&msg);
}

bool ezTestDocumentWindow::HandleEngineMessage(const ezEditorEngineDocumentMsg* pMsg)
{
  if (ezDocumentWindow3D::HandleEngineMessage(pMsg))
    return true;

  if (pMsg->GetDynamicRTTI()->IsDerivedFrom<ezLogMsgToEditor>())
  {
    const ezLogMsgToEditor* pLogMsg = static_cast<const ezLogMsgToEditor*>(pMsg);

    ezLog::Info("Process (%u): '%s'", pLogMsg->m_uiViewID, pLogMsg->m_sText.GetData());

    return true;
  }

  return false;
}



ez3DViewWidget::ez3DViewWidget(QWidget* pParent, ezDocumentWindow3D* pDocument) 
  : QWidget(pParent)
  , m_pDocumentWindow(pDocument)
  , m_MoveContext(this, pDocument->GetDocument(), pDocument)
  , m_SelectionContext(this, pDocument->GetDocument(), pDocument)
{
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAutoFillBackground(false);
  setMouseTracking(true);
}

void ez3DViewWidget::keyReleaseEvent(QKeyEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->keyReleaseEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  if (m_SelectionContext.keyReleaseEvent(e))
    return;

  if (m_MoveContext.keyReleaseEvent(e))
    return;

  QWidget::keyReleaseEvent(e);
}

void ez3DViewWidget::keyPressEvent(QKeyEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->keyPressEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  if (m_SelectionContext.keyPressEvent(e))
    return;

  if (m_MoveContext.keyPressEvent(e))
    return;

  QWidget::keyPressEvent(e);
}

void ez3DViewWidget::mousePressEvent(QMouseEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->mousePressEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  if (m_SelectionContext.mousePressEvent(e))
    return;

  if (m_MoveContext.mousePressEvent(e))
    return;

  QWidget::mousePressEvent(e);
}

void ez3DViewWidget::mouseReleaseEvent(QMouseEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->mouseReleaseEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  if (m_SelectionContext.mouseReleaseEvent(e))
    return;

  if (m_MoveContext.mouseReleaseEvent(e))
    return;

  QWidget::mouseReleaseEvent(e);
}

void ez3DViewWidget::mouseMoveEvent(QMouseEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->mouseMoveEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  if (m_SelectionContext.mouseMoveEvent(e))
    return;

  if (m_MoveContext.mouseMoveEvent(e))
    return;

  QWidget::mouseMoveEvent(e);
}

void ez3DViewWidget::wheelEvent(QWheelEvent* e)
{
  // if a context is active, it gets exclusive access to the input data
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    if (ezEditorInputContext::GetActiveInputContext()->wheelEvent(e))
      return;
  }

  if (ezEditorInputContext::IsAnyInputContextActive())
    return;

  // if no context is active, pass the input through in a certain order, until someone handles it
  if (m_SelectionContext.wheelEvent(e))
    return;

  if (m_MoveContext.wheelEvent(e))
    return;

  QWidget::wheelEvent(e);
}

void ez3DViewWidget::focusOutEvent(QFocusEvent* e)
{
  if (ezEditorInputContext::IsAnyInputContextActive())
  {
    ezEditorInputContext::GetActiveInputContext()->FocusLost();
    ezEditorInputContext::SetActiveInputContext(nullptr);
  }

  QWidget::focusOutEvent(e);
}

ezSelectionContext::ezSelectionContext(QWidget* pParentWidget, ezDocumentBase* pDocument, ezDocumentWindow3D* pDocumentWindow)
  : ezEditorInputContext(pParentWidget, pDocument, pDocumentWindow)
{

}

bool ezSelectionContext::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::MouseButton::LeftButton)
  {
    const ezObjectPickingResult& res = m_pDocumentWindow->PickObject(e->pos().x(), e->pos().y());

    if (e->modifiers() == Qt::KeyboardModifier::AltModifier)
    {
      if (res.m_PickedComponent.IsValid())
      {
        const ezDocumentObjectBase* pObject = m_pDocument->GetObjectTree()->GetObject(res.m_PickedComponent);
        m_pDocument->GetSelectionManager()->SetSelection(pObject);
      }
    }
    else
    {
      if (res.m_PickedObject.IsValid())
      {
        const ezDocumentObjectBase* pObject = m_pDocument->GetObjectTree()->GetObject(res.m_PickedObject);
        m_pDocument->GetSelectionManager()->SetSelection(pObject);
      }
    }

    // we handled the mouse click event
    // but this is it, we don't stay active
    return true;
  }

  return false;
}

bool ezSelectionContext::mouseMoveEvent(QMouseEvent* e)
{
  ezViewHighlightMsgToEngine msg;

  {
    const ezObjectPickingResult& res = m_pDocumentWindow->PickObject(e->pos().x(), e->pos().y());

    if (res.m_PickedComponent.IsValid())
      msg.m_HighlightObject = res.m_PickedComponent;
    else if (res.m_PickedOther.IsValid())
      msg.m_HighlightObject = res.m_PickedOther;
    else
      msg.m_HighlightObject = res.m_PickedObject;
  }

  msg.SendHighlightObjectMessage(m_pDocumentWindow->GetEditorEngineConnection());

  // we only updated the highlight, so others may do additional stuff, if they like
  return false;
}

static const float s_fMoveSpeed[31] =
{
  0.0078125f,
  0.01171875f,
  0.015625f,
  0.0234375f,
  0.03125f,

  0.046875f,
  0.0625f,
  0.09375f,
  0.125f,
  0.1875f,

  0.25f,
  0.375f,
  0.5f,
  0.75f,
  1.0f,

  1.5f,
  2.0f,
  3.0f,
  4.0f,
  6.0f,

  8.0f,
  12.0f,
  16.0f,
  24.0f,
  32.0f,

  48.0f,
  64.0f,
  96.0f,
  128.0f,
  192.0f,

  256.0f,
};

ezCameraMoveContext::ezCameraMoveContext(QWidget* pParentWidget, ezDocumentBase* pDocument, ezDocumentWindow3D* pDocumentWindow)
  : ezEditorInputContext(pParentWidget, pDocument, pDocumentWindow)
{
  m_pCamera = nullptr;
  m_fMoveSpeed = 1.0f;

  m_bRun = false;
  m_bSlowDown = false;
  m_bMoveForwards = false;
  m_bMoveBackwards = false;
  m_bMoveRight = false;
  m_bMoveLeft = false;
  m_bMoveUp = false;
  m_bMoveDown = false;
  m_bMoveForwardsInPlane = false;
  m_bMoveBackwardsInPlane = false;

  m_LastUpdate = ezTime::Now();

  m_bRotateCamera = false;
  m_bMoveCamera = false;
  m_bMoveCameraInPlane = false;
  m_bTempMousePosition = false;


  // do not use SetMoveSpeed here, that would save that value to the settings
  m_iMoveSpeed = 15;
  m_fMoveSpeed = s_fMoveSpeed[m_iMoveSpeed];
}

void ezCameraMoveContext::FocusLost()
{
  m_bRotateCamera = false;
  m_bMoveCamera = false;
  m_bMoveCameraInPlane = false;

  ResetCursor();

  m_bRun = false;
  m_bSlowDown = false;
  m_bMoveForwards = false;
  m_bMoveBackwards = false;
  m_bMoveRight = false;
  m_bMoveLeft = false;
  m_bMoveUp = false;
  m_bMoveDown = false;
  m_bMoveForwardsInPlane = false;
  m_bMoveBackwardsInPlane = false;
}

void ezCameraMoveContext::LoadState()
{
  ezEditorApp::GetInstance()->GetDocumentSettings(m_pDocument->GetDocumentPath(), "TestPlugin").RegisterValueInt("CameraSpeed", 15, ezSettingsFlags::User);
  SetMoveSpeed(ezEditorApp::GetInstance()->GetDocumentSettings(m_pDocument->GetDocumentPath(), "TestPlugin").GetValueInt("CameraSpeed"));
}

void ezCameraMoveContext::UpdateContext()
{
  m_bRun = QApplication::keyboardModifiers() == Qt::KeyboardModifier::ShiftModifier;
  m_bSlowDown = QApplication::keyboardModifiers() == Qt::KeyboardModifier::AltModifier;



  ezTime diff = ezTime::Now() - m_LastUpdate;
  m_LastUpdate = ezTime::Now();

  const double TimeDiff = ezMath::Min(diff.GetSeconds(), 0.1);

  float fSpeedFactor = m_fMoveSpeed * TimeDiff;

  if (m_bRun)
    fSpeedFactor *= 5.0f;
  if (m_bSlowDown)
    fSpeedFactor *= 0.2f;

  if (m_bMoveForwards)
    m_pCamera->MoveLocally(ezVec3(0, 0, -1) * fSpeedFactor);
  if (m_bMoveBackwards)
    m_pCamera->MoveLocally(ezVec3(0, 0, 1) * fSpeedFactor);
  if (m_bMoveRight)
    m_pCamera->MoveLocally(ezVec3(1, 0, 0) * fSpeedFactor);
  if (m_bMoveLeft)
    m_pCamera->MoveLocally(ezVec3(-1, 0, 0) * fSpeedFactor);
  if (m_bMoveUp)
    m_pCamera->MoveGlobally(ezVec3(0, 1, 0) * fSpeedFactor);
  if (m_bMoveDown)
    m_pCamera->MoveGlobally(ezVec3(0, -1, 0) * fSpeedFactor);
  if (m_bMoveForwardsInPlane)
  {
    ezVec3 vDir = m_pCamera->GetCenterDirForwards();
    vDir.y = 0.0f;
    vDir.NormalizeIfNotZero(ezVec3::ZeroVector());
    m_pCamera->MoveGlobally(vDir * fSpeedFactor);
  }
  if (m_bMoveBackwardsInPlane)
  {
    ezVec3 vDir = m_pCamera->GetCenterDirForwards();
    vDir.y = 0.0f;
    vDir.NormalizeIfNotZero(ezVec3::ZeroVector());
    m_pCamera->MoveGlobally(vDir * -fSpeedFactor);
  }
}

bool ezCameraMoveContext::keyReleaseEvent(QKeyEvent* e)
{
  if (!IsActiveInputContext())
    return false;

  if (m_pCamera == nullptr)
    return false;

  //m_bRun = (e->modifiers() == Qt::KeyboardModifier::ShiftModifier);
  //m_bSlowDown = (e->modifiers() == Qt::KeyboardModifier::AltModifier);

  switch (e->key())
  {
  case Qt::Key_W:
    m_bMoveForwards = false;
    return true;
  case Qt::Key_S:
    m_bMoveBackwards = false;
    return true;
  case Qt::Key_A:
    m_bMoveLeft = false;
    return true;
  case Qt::Key_D:
    m_bMoveRight = false;
    return true;
  case Qt::Key_Q:
    m_bMoveUp = false;
    return true;
  case Qt::Key_E:
    m_bMoveDown = false;
    return true;
  case Qt::Key_Left:
    m_bMoveLeft = false;
    return true;
  case Qt::Key_Right:
    m_bMoveRight = false;
    return true;
  case Qt::Key_Up:
    m_bMoveForwardsInPlane = false;
    return true;
  case Qt::Key_Down:
    m_bMoveBackwardsInPlane = false;
    return true;
  }

  return false;
}

bool ezCameraMoveContext::keyPressEvent(QKeyEvent* e)
{
  if (m_pCamera == nullptr)
    return false;

  if (e->modifiers() == Qt::KeyboardModifier::ControlModifier)
    return false;

  //m_bRun = (e->modifiers() == Qt::KeyboardModifier::ShiftModifier);
  //m_bSlowDown = (e->modifiers() == Qt::KeyboardModifier::AltModifier);

  switch (e->key())
  {
    //case Qt::Key_Alt:
    //case Qt::Key_Shift:
    //  SetActiveInputContext(this);
    //  return true;

  case Qt::Key_Left:
    m_bMoveLeft = true;
    SetActiveInputContext(this);
    return true;
  case Qt::Key_Right:
    m_bMoveRight = true;
    SetActiveInputContext(this);
    return true;
  case Qt::Key_Up:
    m_bMoveForwardsInPlane = true;
    SetActiveInputContext(this);
    return true;
  case Qt::Key_Down:
    m_bMoveBackwardsInPlane = true;
    SetActiveInputContext(this);
    return true;
  }

  if (!m_bRotateCamera)
    return false;

  switch (e->key())
  {
  case Qt::Key_W:
    m_bMoveForwards = true;
    return true;
  case Qt::Key_S:
    m_bMoveBackwards = true;
    return true;
  case Qt::Key_A:
    m_bMoveLeft = true;
    return true;
  case Qt::Key_D:
    m_bMoveRight = true;
    return true;
  case Qt::Key_Q:
    m_bMoveUp = true;
    return true;
  case Qt::Key_E:
    m_bMoveDown = true;
    return true;
  }

  return false;
}

bool ezCameraMoveContext::mousePressEvent(QMouseEvent* e)
{
  if (m_pCamera == nullptr)
    return false;

  if (e->button() == Qt::MouseButton::RightButton)
  {
    m_bRotateCamera = true;
    m_LastMousePos = e->globalPos();
    m_bDidMoveMouse[1] = false;
    MakeActiveInputContext();
    return true;
  }

  if (e->button() == Qt::MouseButton::LeftButton)
  {
    m_bMoveCamera = true;
    m_LastMousePos = e->globalPos();
    m_bDidMoveMouse[0] = false;
    MakeActiveInputContext();
    return true;
  }

  if (e->button() == Qt::MouseButton::MiddleButton)
  {
    m_bMoveCameraInPlane = true;
    m_LastMousePos = e->globalPos();
    m_bDidMoveMouse[2] = false;
    MakeActiveInputContext();
    return true;
  }

  return false;
}

void ezCameraMoveContext::ResetCursor()
{
  if (!m_bRotateCamera && !m_bMoveCamera && !m_bMoveCameraInPlane)
  {
    if (m_bTempMousePosition)
    {
      m_bTempMousePosition = false;
      QCursor::setPos(m_OriginalMousePos);
    }

    m_pParentWidget->setCursor(QCursor(Qt::ArrowCursor));
    MakeActiveInputContext(false);
  }
}

void ezCameraMoveContext::SetBlankCursor()
{
  if (m_bRotateCamera || m_bMoveCamera || m_bMoveCameraInPlane)
  {
    m_pParentWidget->setCursor(QCursor(Qt::BlankCursor));
  }
}

void ezCameraMoveContext::SetCursorToWindowCenter()
{
  if (!m_bTempMousePosition)
  {
    m_OriginalMousePos = m_LastMousePos;
    m_bTempMousePosition = true;
  }

  QSize size = m_pParentWidget->size();
  QPoint center;
  center.setX(size.width() / 2);
  center.setY(size.height() / 2);

  center = m_pParentWidget->mapToGlobal(center);

  m_LastMousePos = center;
  QCursor::setPos(center);
}

bool ezCameraMoveContext::mouseReleaseEvent(QMouseEvent* e)
{
  if (!IsActiveInputContext())
    return false;

  if (m_pCamera == nullptr)
    return false;

  if (e->button() == Qt::MouseButton::RightButton)
  {
    m_bRotateCamera = false;

    m_bMoveForwards = false;
    m_bMoveBackwards = false;
    m_bMoveLeft = false;
    m_bMoveRight = false;
    m_bMoveUp = false;
    m_bMoveDown = false;

    ResetCursor();
    return true;
  }

  if (e->button() == Qt::MouseButton::LeftButton)
  {
    m_bMoveCamera = false;
    ResetCursor();

    if (!m_bDidMoveMouse[0])
    {
      // not really handled, so make this context inactive and tell the surrounding code that it may pass
      // the event to the next handler
      return false;

      //const ezObjectPickingResult& res = m_pDocumentWindow->PickObject(e->pos().x(), e->pos().y());

      //if (e->modifiers() == Qt::KeyboardModifier::AltModifier)
      //{
      //  if (res.m_PickedComponent.IsValid())
      //  {
      //    const ezDocumentObjectBase* pObject = m_pDocument->GetObjectTree()->GetObject(res.m_PickedComponent);
      //    m_pDocument->GetSelectionManager()->SetSelection(pObject);
      //  }
      //}
      //else
      //{
      //  if (res.m_PickedObject.IsValid())
      //  {
      //    const ezDocumentObjectBase* pObject = m_pDocument->GetObjectTree()->GetObject(res.m_PickedObject);
      //    m_pDocument->GetSelectionManager()->SetSelection(pObject);
      //  }
      //}
    }

    return true;
  }

  if (e->button() == Qt::MouseButton::MiddleButton)
  {
    m_bMoveCameraInPlane = false;

    ResetCursor();
    return true;
  }

  return false;
}

bool ezCameraMoveContext::mouseMoveEvent(QMouseEvent* e)
{
  // do nothing, unless this is an active context
  if (!IsActiveInputContext())
    return false;

  // store that the mouse has been moved since the last click
  for (ezInt32 i = 0; i < EZ_ARRAY_SIZE(m_bDidMoveMouse); ++i)
    m_bDidMoveMouse[i] = true;

  // send a message to clear any highlight
  ezViewHighlightMsgToEngine msg;
  msg.SendHighlightObjectMessage(m_pDocumentWindow->GetEditorEngineConnection());

  if (m_pCamera == nullptr)
    return false;

  SetBlankCursor();

  float fBoost = 1.0f;

  if (m_bRun)
    fBoost = 5.0f;
  if (m_bSlowDown)
    fBoost = 0.1f;

  const float fAspectRatio = (float)m_pParentWidget->size().width() / (float)m_pParentWidget->size().height();
  const ezAngle fFovX = m_pCamera->GetFovX(fAspectRatio);
  const ezAngle fFovY = m_pCamera->GetFovY(fAspectRatio);

  const float fMouseScale = 4.0f;

  const float fMouseMoveSensitivity = 0.002f * m_fMoveSpeed * fBoost;
  const float fMouseRotateSensitivityX = (fFovX.GetRadian() / (float)m_pParentWidget->size().width()) * fBoost * fMouseScale;
  const float fMouseRotateSensitivityY = (fFovY.GetRadian() / (float)m_pParentWidget->size().height()) * fBoost * fMouseScale;

  if (m_bRotateCamera && m_bMoveCamera) // left & right mouse button -> pan
  {
    const QPointF diff = e->globalPos() - m_LastMousePos;

    float fMoveY = -diff.y() * fMouseMoveSensitivity;
    float fMoveX = diff.x() * fMouseMoveSensitivity;

    m_pCamera->MoveLocally(ezVec3(fMoveX, fMoveY, 0));

    SetCursorToWindowCenter();
    return true;
  }

  if (m_bRotateCamera)
  {
    const QPointF diff = e->globalPos() - m_LastMousePos;

    float fRotateX = diff.y() * -fMouseRotateSensitivityX;
    float fRotateY = diff.x() * -fMouseRotateSensitivityY;

    m_pCamera->RotateGlobally(ezAngle::Radian(0), ezAngle::Radian(fRotateY), ezAngle::Radian(0));
    m_pCamera->RotateLocally(ezAngle::Radian(fRotateX), ezAngle::Radian(0), ezAngle::Radian(0));

    SetCursorToWindowCenter();
    return true;
  }

  if (m_bMoveCamera)
  {
    const QPointF diff = e->globalPos() - m_LastMousePos;

    float fMoveX = diff.x() * fMouseMoveSensitivity;
    float fMoveZ = diff.y() * fMouseMoveSensitivity;

    m_pCamera->MoveLocally(ezVec3(fMoveX, 0, fMoveZ));

    SetCursorToWindowCenter();

    return true;
  }

  if (m_bMoveCameraInPlane)
  {
    const QPointF diff = e->globalPos() - m_LastMousePos;

    float fMoveX = diff.x() * fMouseMoveSensitivity;
    float fMoveZ = -diff.y() * fMouseMoveSensitivity;

    m_pCamera->MoveLocally(ezVec3(fMoveX, 0, 0));

    ezVec3 vDir = m_pCamera->GetCenterDirForwards();
    vDir.y = 0.0f;
    vDir.NormalizeIfNotZero(ezVec3::ZeroVector());

    m_pCamera->MoveGlobally(vDir * fMoveZ);

    SetCursorToWindowCenter();

    return true;
  }

  return false;
}

void ezCameraMoveContext::SetMoveSpeed(ezInt32 iSpeed)
{
  m_iMoveSpeed = ezMath::Clamp(iSpeed, 0, 30);
  m_fMoveSpeed = s_fMoveSpeed[m_iMoveSpeed];

  if (m_pDocument != nullptr)
    ezEditorApp::GetInstance()->GetDocumentSettings(m_pDocument->GetDocumentPath(), "TestPlugin").SetValueInt("CameraSpeed", m_iMoveSpeed);
}

bool ezCameraMoveContext::wheelEvent(QWheelEvent* e)
{
  if (e->modifiers() != Qt::KeyboardModifier::ControlModifier)
    return false;

  if (e->delta() > 0)
  {
    SetMoveSpeed(m_iMoveSpeed + 1);
  }
  else
  {
    SetMoveSpeed(m_iMoveSpeed - 1);
  }

  // handled, independent of whether we are the active context or not
  return true;
}


