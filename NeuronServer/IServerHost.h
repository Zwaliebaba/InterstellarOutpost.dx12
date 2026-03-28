// IServerHost.h — Abstract interface for host-level services that Server needs at runtime.
//
// The application host (e.g., InterstellarOutpost's App) implements this interface,
// wrapping its subsystems (ClientToServer, Multiwinia, GameMenu) behind a narrow contract.
// A headless server provides a minimal/stub implementation.
//
// This decouples NeuronServer from g_app and the InterstellarOutpost/GameLogic projects.
// Created in Wave 5 (Split App) of the client/server library split.

#pragma once

class Directory;

class IServerHost
{
public:
  virtual ~IServerHost() = default;

  // Maximum number of players for the current game map/mode (0 = unlimited).
  virtual int GetMaxNumberOfPlayers() = 0;

  // Whether the game is currently in the lobby phase (pre-game).
  virtual bool IsGameInLobby() = 0;

  // Whether the host is running in single-player mode (local-only, no remote server).
  virtual bool IsLocalSinglePlayer() = 0;

  // The locally-connected client's assigned ID, or -1 if no local client exists.
  virtual int GetLocalClientId() = 0;

  // Deliver a server-to-client letter directly to the local in-process client,
  // bypassing the network socket path. Ownership of _data transfers to callee.
  // No-op if no local client is present (headless server).
  virtual void DeliverToLocalClient(Directory* _data) = 0;

  // Enqueue a letter as if sent by the local client (e.g., spectator assignment).
  // Ownership of _data transfers to callee.
  // No-op if no local client is present (headless server).
  virtual void SendFromLocalClient(Directory* _data) = 0;
};
