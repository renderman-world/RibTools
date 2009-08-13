//==================================================================
/// RibRenderLib_Net_RenderBucketsClient.cpp
///
/// Created by Davide Pasca - 2009/8/6
/// See the file "license.txt" that comes with this project for
/// copyright info. 
//==================================================================

#include "RibRenderLib_Net_RenderBucketsClient.h"

#define NO_LOCAL_RENDERING

//==================================================================
namespace RRL
{
//==================================================================
namespace NET
{

//==================================================================
/// RenderBucketsClient
//==================================================================
RenderBucketsClient::RenderBucketsClient( DVec<Server> &servList ) :
	mpServList(&servList)
{

}

//==================================================================
bool RenderBucketsClient::isAnyServerAvailable() const
{
	for (size_t i=0; i < mpServList->size(); ++i)
	{
		Server &srv = (*mpServList)[i];

		if ( srv.IsConnected() )
			return true;
	}

	return false;
}

//==================================================================
void RenderBucketsClient::Render( RI::HiderREYES &hider )
{
	DUT::QuickProf	prof( __FUNCTION__ );

	const DVec<RI::Bucket *> &buckets = hider.GetBuckets();

	int		bucketsN = (int)buckets.size();

	// loop through the buckets, rendering directly or assigning them
	// to other servers
	for (int buckRangeX1=0; buckRangeX1 < bucketsN; buckRangeX1 += 4)
	{
		// pick a range of 4 buckets
		int buckRangeX2 = DMIN( buckRangeX1 + 4, bucketsN );

		// try to give the buckets to render to the server
	#if defined(NO_LOCAL_RENDERING)
		while NOT( dispatchToServer( buckRangeX1, buckRangeX2 ) )
		{
			// in case one woudl disconnect from all servers..
			if NOT( isAnyServerAvailable() )
			{
				printf( "! No servers available, rendering not possible !\n" );
				return;
			}

			DUT::SleepMS( 10 );
		}
	#else
		if NOT( dispatchToServer( buckRangeX1, buckRangeX2 ) )
		{
			// otherwise render locally..
			//#pragma omp parallel for
			for (int bi=buckRangeX1; bi < buckRangeX2; ++bi)
				RI::FrameworkREYES::RenderBucket_s( hider, *buckets[ bi ] );
		}
	#endif

		checkServersData();
	}

	// tell all servers that we are done rendering
	// ..and they should stop waiting for buckets
	for (size_t i=0; i < mpServList->size(); ++i)
	{
		Server &srv = (*mpServList)[i];

		if ( srv.IsConnected() )
		{
			MsgRendDone	msg;
			srv.mpPakMan->Send( &msg, sizeof(msg) );
		}
	}

	// wait for all servers to complete !!
	while NOT( checkServersData() )
	{
		DUT::SleepMS( 10 );
	}
}

//==================================================================
bool RenderBucketsClient::checkServersData()
{
	bool	allDone = true;

	for (size_t i=0; i < mpServList->size(); ++i)
	{
		Server &srv = (*mpServList)[i];

		if ( !srv.IsConnected() || !srv.mIsBusy )
			continue;

		DNET::Packet *pPacket = srv.mpPakMan->GetNextPacket( false );

		if ( pPacket )
		{
			DASSTHROW(
				pPacket->mDataBuff.size() >= sizeof(U32),
					("Received broken packet from a server !") );

			MsgID	msgID = GetMsgID( pPacket );
			if ( msgID == MSGID_BUCKETDATA )
			{
				srv.mIsBusy = false;

				srv.mpPakMan->RemoveAndDeletePacket( pPacket );
			}
		}
		else
		{
			allDone = false;
		}
	}

	return allDone;
}

//==================================================================
Server * RenderBucketsClient::findFreeServer()
{
	for (size_t i=0; i < mpServList->size(); ++i)
	{
		Server &srv = (*mpServList)[i];

		if ( srv.IsConnected() && !srv.mIsBusy )
			return &srv;
	}

	return NULL;
}

//==================================================================
bool RenderBucketsClient::dispatchToServer( int buckRangeX1, int buckRangeX2 )
{
	Server	*pUseServer = findFreeServer();
	if ( pUseServer )
	{
		MsgRendBuckes	msg;
		msg.BucketStart = (U32)buckRangeX1;
		msg.BucketEnd	= (U32)buckRangeX2;
		pUseServer->mpPakMan->Send( &msg, sizeof(msg) );
		pUseServer->mIsBusy = true;	// set as busy
		return true;
	}
	else
		return false;
}

//==================================================================
}
//==================================================================
}