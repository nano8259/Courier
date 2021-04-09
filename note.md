## 需要修改的地方的记录

### 1. BulkSendApplication

SendData()

属性的话m_maxBytes和m_totBytes可以保留以记录一个流持续了多长（需要保留吗）

然后添加两个map<int, uint32_t>m_maxMfBytes m_totMfBytes用于记录这个流承担的每个Mf的总流量，以及已经发送的流量，发送流量完成后，通知Mf

### 2. macroflow

AggregatedMaper与流不是一对一，而是多对一

### 3. machine-slots

保存到每个机器的BulkSendApplication的map

### 4. master-application

或许不用改？



## 原代码问题

GetCurrentSpeed()之前没有Estimate

好像每个周期都会有estimate一次