// NoiseModuleMessageWnd.h

#ifndef NOISE_MODULE_MESSAGE_WND_H
#define NOISE_MODULE_MESSAGE_WND_H

#define WM_INIT_NOISE_MODULE_THREAD_DATA		WM_USER+7
#define WM_NOISE_MODULE_STATUS_READY			WM_USER+8

class NoiseModule;

class NoiseModuleMessageWnd : public CWnd
{
public:
	void InitParent(NoiseModule *mod);

private:
	NoiseModule *p_mod;

	afx_msg LRESULT InitThreadData(WPARAM wparam,LPARAM lparam);
	void OnTimer(UINT nIDEvent);
	afx_msg LRESULT StatusReady(WPARAM wparam,LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

#endif // NOISE_MODULE_MESSAGE_WND_H