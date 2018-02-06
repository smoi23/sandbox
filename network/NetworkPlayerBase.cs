using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;
using UnityEngine.Events;

namespace bfx
{

	//!
	//! Class to represent a client server connection as a Unity's NetworkBehaviour.
	//! Currently only used to syncronize camera rotation.
	//!
	public class NetworkPlayerBase : NetworkBehaviour
	{
		[SyncVar]
		public Quaternion headVec;
		public float Speed = 4f;

		public VRStatusEvent OnPlayerRestingChanged = new VRStatusEvent();

		private Quaternion cachedHeadVec;
		private bool isResting = false;
		private float restingTimer = 0f;
		private float restingLimit = 10f;

		void Update()
		{
			if (isServer)
			{
				headVec = Camera.main.transform.rotation;
			}
			else
			{
				Camera.main.transform.parent.rotation = Quaternion.Slerp(Camera.main.transform.parent.rotation, headVec, Time.deltaTime * Speed);
			}
		}

		[ClientRpc]
		private void RpcForceRotation(Quaternion rot)
		{
			if (isLocalPlayer) 
			{
				Camera.main.transform.rotation = headVec = rot;
			}
		}

		public void ForceRotationOnClient(Quaternion rot)
		{
			if (isServer)
			{
				RpcForceRotation(rot);
			}
		}

		public override void OnStartLocalPlayer()
		{
			// TODO this is workaround
			NetworkAdapter na = GameObject.FindObjectOfType<NetworkAdapter>();
			if (na != null)
				na.RegisterPlayer(this);

			StartCoroutine(checkValue());
		}

		private IEnumerator checkValue()
		{
			while (true)
			{
				yield return new WaitForSeconds(0.25f);

				if (Quaternion.Angle(headVec, cachedHeadVec) < 0.1f)
				{
					restingTimer += 0.25f;

					if (restingTimer > restingLimit)
					{
						if (!isResting)
						{
							isResting = true;
							OnPlayerRestingChanged.Invoke(isResting);
						}
					}
				}
				else
				{
					restingTimer = 0f;
					if (isResting)
					{
						isResting = false;
						OnPlayerRestingChanged.Invoke(isResting);
					}
				}

				cachedHeadVec = headVec;

			}
		}

		void OnDisable()
		{
			StopAllCoroutines();
		}
	}
}