// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� ALGOLIB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// ALGOLIB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef ALGOLIB_EXPORTS
#define ALGOLIB_API __declspec(dllexport)
#else
#define ALGOLIB_API __declspec(dllimport)
#endif

// �����Ǵ� Algolib.dll ������
class ALGOLIB_API CAlgolib {
public:
	CAlgolib(void);
	// TODO:  �ڴ�������ķ�����
};

extern ALGOLIB_API int nAlgolib;

ALGOLIB_API int fnAlgolib(void);
