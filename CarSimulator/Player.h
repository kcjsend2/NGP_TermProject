#pragma once

#define KEY_FORWARD 0x01
#define KEY_BACKWARD 0x02
#define KEY_LEFT 0x04
#define KEY_RIGHT 0x08
#define KEY_UP 0x10
#define KEY_DOWN 0x20
#define KEY_SHIFT 0x40
#define KEY_X 0x80


#include "GameObject.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
protected:
	//플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터이다.
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다.
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	//플레이어의 이동 속도를 나타내는 벡터이다.
	XMFLOAT3 m_xmf3Velocity;

	//플레이어에 작용하는 중력을 나타내는 벡터이다.
	XMFLOAT3 m_xmf3Gravity;

	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float m_fMaxVelocityXZ;

	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float m_fMaxVelocityY;

	//플레이어에 작용하는 마찰력을 나타낸다.
	float m_fFriction;

	//플레이어의 최대 속력
	float m_fMaxVelocity = 5.0f;

	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdateCallback() 함수에서 사용하는 데이터이다.
	LPVOID m_pPlayerUpdatedContext;

	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdateCallback() 함수에서 사용하는 데이터이다.
	LPVOID m_pCameraUpdatedContext;

	//플레이어에 현재 설정된 카메라이다.
	CCamera *m_pCamera = NULL;

public:
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();
	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }
	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	/*플레이어의 위치를 xmf3Position 위치로 설정한다. xmf3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현
	재 플레이어의 위치에서 xmf3Position 방향으로의 벡터가 된다. 현재 플레이어의 위치에서 이 벡터 만큼 이동한다.*/

	void SetPosition(XMFLOAT3& xmf3Position)
	{
		Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false);
	}

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }

	float GetYaw() { return(m_fYaw); }
	float GetPitch() { return(m_fPitch); }
	float GetRoll() { return(m_fRoll); }
	CCamera* GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }

	//플레이어를 이동하는 함수이다.
	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	//플레이어를 회전하는 함수이다.
	void Rotate(float x, float y, float z);
	void Rotate(DWORD dwDirection);

	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다.
	void Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	//카메라를 변경하기 위하여 호출하는 함수이다.
	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
	{
		return(NULL);
	}

	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다.
	virtual void OnPrepareRender();
	//플레이어의 카메라가 3인칭 카메라일 때 플레이어(메쉬)를 렌더링한다.
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	int m_nLife = 3;
};

class CVehiclePlayer : public CPlayer
{
private:
	class CWheel : public CGameObject
	{
	public:
		CWheel(std::shared_ptr<CMeshFileRead> pWheelMesh);
		~CWheel();
		virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
		virtual void Update(float fTimeElapsed, btRaycastVehicle* pbtVehicle, int index);
	};

public:
	CVehiclePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, btAlignedObjectArray<btCollisionShape*>& btCollisionShapes, btDiscreteDynamicsWorld* pbtDynamicsWorld, int nMeshes = 5);
	virtual ~CVehiclePlayer();

	virtual void Update(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fTimeElapsed, btDiscreteDynamicsWorld* pbtDynamicsWorld, DWORD dwBehave);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	void FireBullet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, btDiscreteDynamicsWorld* pbtDynamicsWorld);
	void SetSpawPosition(XMFLOAT3 xmf3Position);
	XMFLOAT3 GetSpawnPosition() { return m_xmf3SpawnPosition; }
	void EraseBullet() { m_pBullet = NULL; }
	void SetNextFrameMessage(int msg) { m_nNextFrameMsg = msg; }
	std::shared_ptr<CBullet> GetBullet() { return m_pBullet; };
	std::shared_ptr<CWheel>* GetWheels() { return m_pWheel; }
	btRaycastVehicle* GetVehicle() { return m_vehicle; }
	void SetRigidBodyPosition(XMFLOAT3 xmf3Position);

private:
	std::shared_ptr<CWheel> m_pWheel[4];

	XMFLOAT3 m_xmf3SpawnPosition;

	btRaycastVehicle::btVehicleTuning m_tuning;
	btVehicleRaycaster* m_vehicleRayCaster;
	btRaycastVehicle* m_vehicle;

	std::shared_ptr<CBullet> m_pBullet;

	int m_nNextFrameMsg = 0;

	float m_gEngineForce = 0.f;

	float m_defaultBreakingForce = 10.f;
	float m_gBreakingForce = 0.f;

	float m_maxEngineForce = 4000.f;
	float m_EngineForceIncrement = 5.0f;

	float m_gVehicleSteering = 0.f;
	float m_steeringIncrement = 0.01f;
	float m_steeringClamp = 0.1f;
	float m_wheelRadius = 0.5f;
	float m_wheelWidth = 0.4f;

// 게임, 네트워크와 관련된 변수와 함수는 아래에 작성한다.
private:
	int m_life = 3;

public:
	//int GetLife() const { return m_life; }
};
