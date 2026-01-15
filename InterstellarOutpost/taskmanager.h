#ifndef _included_taskmanager_h
#define _included_taskmanager_h

#include "llist.h"
#include "routing_system.h"
#include "worldobject.h"
#include "unicode_string.h"

#define POWERUP_EFFECT_RANGE 70.0

// class Task

class Task
{
  public:
    enum
    {
      StateIdle,
      // Not yet run
      StateStarted,
      // Running, not yet targetted
      StateRunning,
      // Targetted, running
      StateStopping // Stopping
    };

    int m_teamId;
    int m_id;
    int m_type;
    int m_state;
    WorldObjectId m_objId;
    Vector3 m_pos; // used to track the click position of the airstrike task, and similar

    Route* m_route; // Only used when this is a Controller task

    float m_lifeTimer; // Tracks the lifetime of time-limited tasks such as booster
    float m_startTimer; // If this is greater than 0, the task cannot be used
    int m_option; // used to track an extra variable, currently only used for gunturret states

    float m_screenX; // Used in the interface
    float m_screenY;
    float m_screenH;

    Task(int _teamId);
    ~Task();

    void Start();
    bool Advance();
    void SwitchTo();
    void Stop();

    void Target(const Vector3& _pos);
    void TargetSquad(const Vector3& _pos);
    void TargetEngineer(const Vector3& _pos);
    void TargetOfficer(const Vector3& _pos);
    void TargetArmour(const Vector3& _pos);
    void TargetHarvester(const Vector3& _pos);
    void TargetAirStrike(const Vector3& _pos);
    void TargetNuke(const Vector3& _pos);
    void TargetSubversion(const Vector3& _pos);
    void TargetBooster(const Vector3& _pos);
    void TargetHotFeet(const Vector3& _pos);
    void TargetGunTurret(const Vector3& _pos);
    void TargetShaman(const Vector3& _pos);
    void TargetInfection(const Vector3& _pos);
    void TargetMagicalForest(const Vector3& _pos);
    void TargetDarkForest(const Vector3& _pos);
    void TargetAntNest(const Vector3& _pos);
    void TargetPlague(const Vector3& _pos);
    void TargetEggs(const Vector3& _pos);
    void TargetMeteorShower(const Vector3& _pos);
    void TargetExtinguisher(const Vector3& _pos);
    void TargetRage(const Vector3& _pos);
    void TargetTank(const Vector3& _pos);
    void TargetDropShip(const Vector3& _pos);

    bool ShutdownController(); // Squaddies stopping their control of darwinians

    Vector3 GetPosition();

    WorldObjectId Promote(WorldObjectId _id);
    static WorldObjectId Demote(WorldObjectId _id);
    static WorldObjectId FindDarwinian(const Vector3& _pos, int _teamId);

    static char* GetTaskName(int _type);
    static void GetTaskNameTranslated(int _type, UnicodeString& _dest);
};

// class TaskTargetArea

class TaskTargetArea
{
  public:
    Vector3 m_centre;
    float m_radius;
    bool m_stationary;
};

// class TaskSource

class TaskSource
{
  public:
    int m_taskId;
    double m_timer;
    int m_source; // 0=crate, 1=reinforcement, 2=retribution
    float m_rotation;

    TaskSource()
      : m_taskId(-1),
        m_timer(0),
        m_source(0),
        m_rotation(0) {}
};

// class TaskManager

class TaskManager
{
  public:
    LList<Task*> m_tasks;

    int m_teamId;
    int m_nextTaskId;
    int m_currentTaskId;
    bool m_verifyTargetting;

    int m_mostRecentTaskId;
    double m_mostRecentTaskIdTimer;
    bool m_blockMostRecentPopup;

    DArray<TaskSource*> m_recentTasks;

    int m_previousTaskId;

    WorldObjectId m_mostRecentEntity; // assigned to the id of the most recent entity created *note* currently only works for officers

    TaskManager(int _teamId);

    int Capacity();
    int CapacityUsed();

    bool RunTask(Task* _task); // Starts the task, registers it
    bool RunTask(int _type);
    bool RegisterTask(Task* _task); // Assumes task is already started, just registers it

    Task* GetCurrentTask();
    Task* GetTask(int _id);
    Task* GetTask(WorldObjectId _id);
    void SelectTask(int _id);
    void SelectTask(WorldObjectId _id); // Selects the corrisponding task 
    bool IsValidTargetArea(int _id, const Vector3& _pos);
    bool TargetTask(int _id, const Vector3& _pos);
    bool TerminateTask(int _id, bool _showMessage = true);

    int GetTaskIndex(int _id); // Use carefully, indexes can change at any time
    int GetNextTaskId();

    int CountNumTasks(int _type);

    void NotifyRecentTaskSource(int source);

    void AdvanceTasks();
    void StopAllTasks();

    LList<TaskTargetArea>* GetTargetArea(int _id); // Returns all valid placement areas

    void Advance();

    static int MapGestureToTask(int _gestureId); // Maps a gesture to a task type
};

#endif
