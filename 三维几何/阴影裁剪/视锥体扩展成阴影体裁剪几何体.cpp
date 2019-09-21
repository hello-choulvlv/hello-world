// �ж�����ƽ��Ľ����Ƿ���������   
int TestSilhouette(const Plane& p0, const Plane& p1, const Vector3& dir)
{
    float t0 = p0.m_Normal.Dot(dir);  
    float t1 = p1.m_Normal.Dot(dir);  
    if (t0 * t1 > 0.0f)  
    {
        return 0;
    }

    return t0 > 0.0f ? 1 : 2;
}
// �㷨�����ݹ�Դ���������׶��������ߣ��������ߺ͹�Դ�������Χƽ��   
// ��Χƽ��������������Դ����ƽ��   
void CreateLightCullingVolume(Camera* pCamera, const Vector3& lightDir, /*out*/vector<Plane>& cullingPlanes)  
{
    // 12���������������˳��Ϊ����ƽ��˳ʱ��4���⣬   
    // Զƽ��˳ʱ��4����,Զ��ƽ��֮��˳ʱ��4����   
    static unsigned short index[] =
    {
        1, 0, 0, 2, 2, 3, 3, 1,
        5, 4, 4, 6, 6, 7, 7, 5,
        1, 5, 0, 4, 2, 6, 3, 7,
    };
  
    // ÿ���ⶼ��������ƽ���ཻ���ɵ�,�����������Ӧ��12��ƽ��,   
    // ÿ������ÿ��ƽ���ϵĵ�һ��ƽ������˳��ģ��ڵڶ���ƽ�����������   
    // �����һ���⣨0��1������ƽ������������⣩�ڵ�һ��ƽ��FP_Near����˳���,������ϵĴָ�����ߣ�,   
    // �ڵڶ���ƽ��FP_Top���������   
    static EFrustumPlane planePair[][2] =
    {
        {FP_Near, FP_Top}, {FP_Near, FP_Right}, {FP_Near, FP_Bottom}, {FP_Near, FP_Left},   
        {FP_Top, FP_Far}, {FP_Right, FP_Far}, {FP_Bottom, FP_Far}, {FP_Left, FP_Far},   
        {FP_Top, FP_Left}, {FP_Right, FP_Top}, {FP_Bottom, FP_Right}, {FP_Left, FP_Bottom},  
    };  

    const Vector3* pCorner = pCamera->GetFrustumCorners();  
    // ���β���12�����Ƿ�Ϊ������   
    for (int i = 0; i < 12; i++)  
    {
        int testSilhouette = TestSilhouette(pCamera->GetPlane(planePair[i][0]), pCamera->GetPlane(planePair[i][1]), lightDir);
        // ��������
        if (testSilhouette > 0)
        {
            // ����ǵ�һ��ƽ�棬����˳�ŵģ�����ǵڶ���ƽ�棬�������ŵ�   
            Vector3 edgeDir;  
            if (testSilhouette == 1)  
            {
                edgeDir = pCorner[index[i * 2 + 1]] - pCorner[index[i * 2 + 0]];
            }
            else
            {
                edgeDir = pCorner[index[i * 2 + 0]] - pCorner[index[i * 2 + 1]];  
            }  
            // �������߹����޳�ƽ�棬���߳���   
            Vector3 normal = lightDir.Cross(edgeDir);
            normal.Normalize();
  
            // �����߱������������޳�   
            m_LightPlanes.push_back(Plane(normal, pCorner[index[i * 2 + 0]]));
        }
    }

    //��׶ƽ���г����Դ��Ҳ�����޳�ƽ��,��׼ȷ��˵�Ǳ����Դ��
    for (int i = 0; i < 6; i++)
    {
        const Plane& plane = pCamera->GetPlane((EFrustumPlane)i);
        if (plane.m_Normal.Dot(lightDir) > 0.0f)
        {
            vector<Plane>.push_back(plane);
        }
    }
}