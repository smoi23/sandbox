using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.Networking;





namespace bfx
{

	//!
	//! Main class to handle network connections using Unity's high level api. 
	//! Start as server or client, send or listen for broadcastings, contains method to send or receive VR messages.
	//!
	public class NetworkAdapter : NetworkDiscovery
	{
		[SerializeField]
		public GameObject userNotPresent;

		public VRCommandEvent OnVRCommandEvent = new VRCommandEvent();


		public UnityEvent OnClientConnectedEvent = new UnityEvent();


		public UnityEvent OnClientDisconnectedEvent = new UnityEvent();

		private NetworkManagerBase networkManager;
		private List<string> serverList = new List<string>();
		public List<string> ServerList
		{
			get { return serverList; }
		}

		public static short MSG_vrcmd = 664;

		private GameObject vrServerListRoot = null;
		private GameObject master = null;

		public VRMessage[] clientInitMessageList = { null, null, null, null }; // {journey, view, fade, vrbuttonclick}

		//!
		//! Indicate when main application is ready.
		//!
		public bool IgnoreBroadcast = true;

		//!
		//! Use to build
		//!
		void Awake()
		{
			networkManager = gameObject.GetComponent<NetworkManagerBase>();

			if (networkManager == null) Debug.LogError(string.Format("{0}: Cant get: NetworkManager.", this.GetType()));

			// call initialize at NetworkDiscovery to avoid error
			Initialize();

		}

		//!
		//! Use to initialize
		//!
		void Start()
		{
			vrServerListRoot = GameObject.Find("vrServerListRoot");

			master = GameObject.Find("master");
			if (master)
				master.SetActive(false);

			//userNotPresent = GameObject.Find ("userNotPresent");
			if (userNotPresent)
				userNotPresent.SetActive(false);

			if (networkManager != null)
			{
				#if (IS_SERVER)
				// Register for new connected client event
				networkManager.OnServerConnectEvent.AddListener(handleServerConnected);
				networkManager.StartServer();
				print("Start Discovery as Server. Broadcasting address.");
				StartAsServer();
				#else
				networkManager.OnClientConnectEvent.AddListener(handleClientConnected);
				networkManager.OnClientReconnectEvent.AddListener(handleClientReconnected);
				networkManager.OnClientDisconnectEvent.AddListener(handleClientDisconnected);
				StartListenAddress();
				#endif
			}
		}


		//!
		//! Callback on server side when connected to client
		//! Send intial messages to new connected client
		//! @param		msg		the network connection to the client
		//!
		private void handleServerConnected(NetworkConnection conn)
		{
			foreach (VRMessage msg in clientInitMessageList)
			{
				if (msg != null) NetworkServer.SendToClient(conn.connectionId, MSG_vrcmd, msg);
			}
		}

		//!
		//! Toggle receiving broadcast messages
		//!
		public void ToggleListenAddress()
		{
			if (running)
				StopListenAddress();
			else
				StartListenAddress();
		}

		//!
		//! If client start receiving broadcast messages
		//!
		public void StartListenAddress()
		{
			if (!isServer && !running)
			{
				// call initialize at NetworkDiscovery to avoid error
				Initialize();
				print("Start Discovery as Client and listen for address");
				StartAsClient();
				StopCoroutine(TestIPAddress());
				StartCoroutine(TestIPAddress());
			}
		}

		//!
		//! If client stop receiving broadcast messages
		//!
		public void StopListenAddress()
		{
			if (!isServer && running)
			{
				print("Stop Broadcast");
				StopCoroutine(TestIPAddress());
				StopBroadcast();
			}
		}

		//!
		//! Connect as client to server and register callback for network messages of type 'MSG_vrcmd'. If already connected, disconnect first.
		//!
		public void ConnectToServer(string toAddress, int toPort = 7777)
		{

			int port = toPort != 0 ? toPort : 7777;

			if (!isServer)
			{
				networkManager.networkAddress = toAddress;
				networkManager.networkPort = port;
				if (networkManager.IsClientConnected())
				{
					networkManager.client.Disconnect();
					print("ReConnectTo: " + toAddress + ":" + port);
					networkManager.client.Connect(toAddress, port);
				}
				else
				{
					print("Start Client on IP: " + toAddress + ":" + port);
					networkManager.StartClient();
					// register for network messages
					networkManager.client.RegisterHandler(NetworkAdapter.MSG_vrcmd, OnVRCommand);
				}

				setNGUIStatus(toAddress, ConnectionStatus.CONNECTING);
			}
		}

		//!
		//! Callback on client side when connected to server
		//! @param		msg		the network connection to the server
		//!
		private void handleClientConnected(NetworkConnection conn)
		{
			setNGUIStatus(conn.address, ConnectionStatus.CONNECT);
			// start check routine if player was added and show message if was not
			StartCoroutine(WaitAndTestPlayers());
			// simply forward
			OnClientConnectedEvent.Invoke();
		}


		//!
		//! Callback on client side when Re-connected to server
		//! @param		msg		the network connection to the server
		//!
		private void handleClientReconnected(NetworkConnection conn)
		{
			if (conn.lastError == NetworkError.Ok)
				setNGUIStatus(conn.address, ConnectionStatus.CONNECTING);
			else
				setNGUIStatus(conn.address, ConnectionStatus.ERROR_RECONNECT);
		}

		//!
		//! Callback on client side when disconnected from server
		//! @param		msg		the network connection to the server
		//!
		private void handleClientDisconnected(NetworkConnection conn)
		{
			setNGUIStatus(conn.address, ConnectionStatus.LOST);
		}


		//!
		//! Called from spawned players when client is connected and after ownership message from server arrived
		//!
		public void RegisterPlayer(NetworkPlayerBase player)
		{
			if (userNotPresent)
				userNotPresent.SetActive(false);
			player.OnPlayerRestingChanged.AddListener(handlePlayerResting);
		}


		//!
		//! This waits and check if a player was actually added. A connection is established but for example the server app is suspended no player is spawned
		//!
		private IEnumerator WaitAndTestPlayers()
		{
			yield return new WaitForSeconds(1f);

			if (networkManager.client.connection.playerControllers.Count == 0)
			{
				if (userNotPresent)
					userNotPresent.SetActive(true);
			}
		}

		//!
		//! Callback on client side when player is resting, i.e. no value changes
		//!
		private void handlePlayerResting(bool status)
		{
			if (userNotPresent)
				userNotPresent.SetActive(status);
		}


		//!
		//! If server send a message to all clients
		//! @param		msg		message to be send
		//!
		public void SendVRMessageToAll(VRMessage msg)
		{
			if (isServer)
				NetworkServer.SendToAll(MSG_vrcmd, msg);
		}

		//!
		//! On all spawned players call ForceRotation
		//! TODO make a generic function
		//! @param		rot		rotation to be set
		//!
		public void RpcCallToAll(Quaternion rot)
		{
			foreach (KeyValuePair<NetworkInstanceId, NetworkIdentity> pair in NetworkServer.objects)
			{
				if (pair.Value.gameObject != null)
				{
					NetworkPlayerBase player = pair.Value.gameObject.GetComponent<NetworkPlayerBase>();
					if (player != null)
						player.ForceRotationOnClient(rot);
				}
			}
		}

		//!
		//! Halt connections
		//!
		public void Disconnect()
		{
			if (!isServer)
			{
				if (networkManager.client != null)
					setNGUIStatus(networkManager.client.serverIp, ConnectionStatus.DISCONNECT);
				else
					setNGUIStatus("0.0.0.0", ConnectionStatus.DISCONNECT);
				networkManager.StopClient();
				if (userNotPresent)
					userNotPresent.SetActive(false);
				// forward
				OnClientDisconnectedEvent.Invoke();
			}
			else
			{
				networkManager.StopServer();
			}

		}

		//!
		//! Clear address list and start listening
		//!
		public void Refresh()
		{
			serverList.Clear();
			if (vrServerListRoot != null)
			{
				foreach(Transform child in vrServerListRoot.transform)
				{
					if (networkManager.client != null && child.name == networkManager.client.serverIp)
						continue;

					child.parent = null;
					UnityEngine.Object.Destroy(child.gameObject);
				}
			}
			StartListenAddress();
		}

		//!
		//! Overide. Disconnect on quit.
		//!
		void OnApplicationQuit()
		{
			StopListenAddress();
			Disconnect();
		}

		//!
		//! Overide. Broadcast receive event. Strips IP address from data and add it to list.
		//! @param 		fromAddress		port, ip as string
		//! @param 		data			broadcast content
		//!
		public override void OnReceivedBroadcast(string fromAddress, string data)
		{
			if (IgnoreBroadcast)
				return;

			// Debug.Log("ReceivedBroadcast from "+fromAddress+" (" + data + ")");

			var adressTmp = fromAddress.Split(':');
			string address = adressTmp[3];

			if (!serverList.Contains(address))
			{
				serverList.Add(address);
				addNGUIItem(address);
			}
		}

		//!
		//! Registered callback for network messages of type 'MSG_vrcmd'.
		//! @param 		netMsg		message object
		//!
		public void OnVRCommand(NetworkMessage netMsg)
		{
			if (!isServer)
			{
				VRMessage msg = netMsg.ReadMessage<VRMessage>();
				OnVRCommandEvent.Invoke(msg);
			}
		}

		private IEnumerator TestIPAddress()
		{
			while (true)
			{
				yield return new WaitForSeconds(1f);

				if (vrServerListRoot != null)
				{
					foreach (Transform child in vrServerListRoot.transform)
					{
						Color color = new Color(0.74f, 0.82f, 0.64f);

						if (!serverList.Contains(child.name))
							color = new Color(0.80f, 0.41f, 0.16f);

						if (child.GetComponent<UISprite>() != null)
							child.GetComponent<UISprite>().color = color;
					}
				}

				serverList.Clear();
			}
		}


		//!
		//! Add a new label object and set text to value
		//! @param		value		string to be set
		//!
		private void addNGUIItem(string value)
		{
			if (vrServerListRoot != null && master != null && vrServerListRoot.transform.Find(value) == null )
			{
				GameObject tmpEntry = NGUITools.AddChild(vrServerListRoot, master);
				tmpEntry.name = value;
				NGUITools.SetActive(tmpEntry, true);
				tmpEntry.transform.GetComponentInChildren<UILabel>().text = value;
				vrServerListRoot.GetComponent<UIGrid>().Reposition();
			}
		}


		private void setNGUIStatus(string address, ConnectionStatus status)
		{
			Color color = new Color(0.82f, 0.82f, 0.82f);
			string text = "Disconnected";

			switch (status)
			{
			case ConnectionStatus.CONNECT:
				color = new Color(0.74f, 0.82f, 0.64f);
				text = "Connected";
				break;
			case ConnectionStatus.CONNECTING:
				text = "Connecting";
				break;
			case ConnectionStatus.ERROR_RECONNECT:
				color = new Color(0.86f, 0.80f, 0.27f);
				text = "Reconnecting";
				break;
			case ConnectionStatus.LOST:
				color = new Color(0.80f, 0.41f, 0.16f);
				text = "Lost";
				break;
			default:
				break;
			}

			if (vrServerListRoot != null)
			{
				foreach (Transform child in vrServerListRoot.transform)
				{
					if (child.name == address || address == "0.0.0.0")
					{
						Transform statusObject = child.Find("status");
						if (statusObject != null)
						{
							if (statusObject.GetComponent<UISprite>() != null)
								statusObject.GetComponent<UISprite>().color = color;

							if (statusObject.GetComponentInChildren<UILabel>() != null)
								statusObject.GetComponentInChildren<UILabel>().text = text;
						}
					}
				}
			}

		}
	}

	//!
	//! Class to define message structure used to send via networkServer.SendToAll.
	//!
	public class VRMessage : MessageBase
	{
		public string cmd;
		public string content = "";
		public VRMessage() : base() { }

		public VRMessage(string c = null)
		{
			cmd = c;
		}
	}

	//!
	//! Class to define event type accepting VRMessage as argument.
	//!
	public class VRCommandEvent : UnityEvent<VRMessage> { };

	//!
	//! Class to define event type accepting boolean as argument.
	//!
	public class VRStatusEvent : UnityEvent<bool> { };

	public enum ConnectionStatus { CONNECT, DISCONNECT, CONNECTING, RECONNECTING, ERROR_RECONNECT, ERROR_TIMEOUT, AVAILABLE, LOST };

}