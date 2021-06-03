#include "DriverEntry.h"

#define HTTP_DEFAULT_PORT 80

extern "C"
{
	PCHAR PsGetProcessImageFileName(PEPROCESS Process); // 获取内核对象的进程名称

	CHAR* DirectionName = NULL; CHAR* ProtocalName = NULL;
	void Wfp_Sample_Established_ClassifyFn_V4(
		_In_ const FWPS_INCOMING_VALUES* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT* classifyOut
	)
	{
		WORD	wDirection = 0;
		WORD	wRemotePort = 0;
		WORD	wSrcPort = 0;
		WORD	wProtocol = 0;
		ULONG	ulSrcIPAddress = 0;
		ULONG	ulRemoteIPAddress = 0;

		if (!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE))
		{
			return;
		}
		//wDirection表示数据包的方向,取值为	//FWP_DIRECTION_INBOUND/FWP_DIRECTION_OUTBOUND
		wDirection = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_DIRECTION].value.int8;

		//wSrcPort表示本地端口，主机序
		wSrcPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_PORT].value.uint16;

		//wRemotePort表示远端端口，主机序
		wRemotePort = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_PORT].value.uint16;

		//ulSrcIPAddress 表示源IP
		ulSrcIPAddress = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_ADDRESS].value.uint32;

		//ulRemoteIPAddress 表示远端IP
		ulRemoteIPAddress = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_ADDRESS].value.uint32;

		//wProtocol表示网络协议，可以取值是IPPROTO_ICMP/IPPROTO_UDP/IPPROTO_TCP
		wProtocol = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_PROTOCOL].value.uint8;

		//默认"允许"(PERMIT)
		classifyOut->actionType = FWP_ACTION_PERMIT;
		
		//简单的策略判断，可以重写这部分
		if ((wProtocol == IPPROTO_TCP) &&
			(wDirection == FWP_DIRECTION_OUTBOUND) &&
			(wRemotePort == HTTP_DEFAULT_PORT))
		{
			//TCP协议尝试发起80端口的访问，拦截(BLOCK)
			classifyOut->actionType = FWP_ACTION_BLOCK;

			/*
				填写 YOUS CODE...
			*/
		
			switch (wDirection)
			{
			case FWP_DIRECTION_INBOUND:
				DirectionName = "Recv"; break;
			case FWP_DIRECTION_OUTBOUND:
				DirectionName = "Send"; break;
			}
		
			switch (wProtocol)
			{
			case IPPROTO_ICMP:
				ProtocalName = "ICMP"; break;
			case IPPROTO_UDP:
				ProtocalName = "UDP";  break;
			case IPPROTO_TCP:
				ProtocalName = "TCP";  break;
			}

			DbgPrint("[%s]--%s:[%s], LocalIp=[%u.%u.%u.%u:%d], RemoteIp=[%u.%u.%u.%u:%d], PID=[%I64d]\r\n",
				PsGetProcessImageFileName(IoGetCurrentProcess()),
				DirectionName,
				ProtocalName,
				(ulSrcIPAddress >> 24) & 0xFF,
				(ulSrcIPAddress >> 16) & 0xFF,
				(ulSrcIPAddress >> 8) & 0xFF,
				(ulSrcIPAddress) & 0xFF,
				wSrcPort,
				(ulRemoteIPAddress >> 24) & 0xFF,
				(ulRemoteIPAddress >> 16) & 0xFF,
				(ulRemoteIPAddress >> 8) & 0xFF,
				(ulRemoteIPAddress) & 0xFF,
				wRemotePort,
				inMetaValues->processId
			);
		}

		//清除FWPS_RIGHT_ACTION_WRITE标记
		if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
		{
			classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		}
		return;
	}

	NTSTATUS NTAPI Wfp_Sample_Established_NotifyFn_V4(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER* filter
	)
	{
		return STATUS_SUCCESS;
	}

	void NTAPI Wfp_Sample_Established_FlowDeleteFn_V4(
		_In_ UINT16 layerId,
		_In_ UINT32 calloutId,
		_In_ UINT64 flowContext)
	{
		return;
	}
}


