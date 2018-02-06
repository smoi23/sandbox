using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.Networking;

namespace bfx
{

	public class ServerConnectEvent : UnityEvent<NetworkConnection> {};
    public class ClientConnectEvent : UnityEvent<NetworkConnection> { };

    public class NetworkManagerBase : NetworkManager
	{
		public bool doAutoReconnect = true;

		public ServerConnectEvent OnServerConnectEvent = new ServerConnectEvent();
        public ClientConnectEvent OnClientConnectEvent = new ClientConnectEvent();
        public ClientConnectEvent OnClientReconnectEvent = new ClientConnectEvent();
        public ClientConnectEvent OnClientDisconnectEvent = new ClientConnectEvent();

		public override void OnServerConnect(NetworkConnection conn)
		{
			print("[NetworkManagerBase::OnServerConnect]");
			OnServerConnectEvent.Invoke(conn);
			base.OnServerConnect(conn);
		}


		public override void OnServerDisconnect(NetworkConnection conn)
		{
			if (conn.lastError == NetworkError.Timeout)
			{
				Debug.LogWarning("Client(" + conn.connectionId + ") disconnected due: " + conn.lastError );
				NetworkServer.DestroyPlayersForConnection(conn);
			}
			else
			{
				base.OnServerDisconnect(conn);
			}
		}

		public override void OnClientConnect(NetworkConnection conn)
		{
			print("[NetworkManagerBase::OnClientConnect]");
            OnClientConnectEvent.Invoke(conn);
			base.OnClientConnect(conn);
		}

		public override void OnClientDisconnect(NetworkConnection conn)
		{
            Debug.LogWarning("[NetworkManagerBase::OnClientDisconnect] ConnectionError: " + conn.lastError);
            if (conn.lastError == NetworkError.Timeout && doAutoReconnect)
			{
				StopAllCoroutines();
				StartCoroutine(WaitAndConnect(1f));
                OnClientReconnectEvent.Invoke(conn);
			}
			else
			{
                OnClientDisconnectEvent.Invoke(conn);
				base.OnClientDisconnect(conn);
			}
		}

		IEnumerator WaitAndConnect(float delay)
		{
			yield return new WaitForSeconds(delay);
			if (!IsClientConnected() && client != null)
			{
				client.Connect(networkAddress, networkPort);
			}
			yield return null;
		}
	}
}