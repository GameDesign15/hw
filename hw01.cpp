/*==============================================================
character movement testing using Fly2

- Load a scene
- Generate a terrain object
- Load a character
- Control a character to move
- Change poses

(C)2012-2015 Chuan-Chang Wang, All Rights Reserved
Created : 0802, 2012

Last Updated : 1004, 2015, Kevin C. Wang
===============================================================*/
#include "FlyWin32.h"
#include <cmath>


VIEWPORTid vID;                 // the major viewport
SCENEid sID;                    // the 3D scene
OBJECTid cID, tID;              // the main camera and the terrain for terrain following
CHARACTERid actorID, donzoID, robberID;            // the major character
ACTIONid idleID, runID, curPoseID, walkID, idle2ID, run2ID,
curPose2ID, walk2ID, idle3ID, run3ID, curPose3ID, walk3ID, DieID, Die2ID, Die3ID, AttactID, DamageLID, DamageID;
ROOMid terrainRoomID = FAILED_ID;
TEXTid textID = FAILED_ID;

// some globals
int frame = 0;
float constant_distant;
int oldX, oldY, oldXM, oldYM, oldXMM, oldYMM;
float judge_dir = 0;
float adjust = 2.0f;
float donzoblood = 10.0f;
float robberblood = 20.0f;

// hotkey callbacks
void QuitGame(BYTE, BOOL4);
void Movement(BYTE, BOOL4);
void Movement2(BYTE, BOOL4);
void Attact(BYTE, BOOL4);

float AttactSys(float, int);
// timer callbacks
void GameAI(int);
void RenderIt(int);

// mouse callbacks
void InitPivot(int, int);
void PivotCam(int, int);
void InitMove(int, int);
void MoveCam(int, int);
void InitZoom(int, int);
void ZoomCam(int, int);

/*------------------
the main program
C.Wang 1010, 2014
-------------------*/
void FyMain(int argc, char **argv)
{
	// create a new world
	BOOL4 beOK = FyStartFlyWin32("NTU@2014 Homework #02 - Use Fly2", 0, 0, 1024, 768, FALSE);

	// setup the data searching paths
	FySetShaderPath("Data\\NTU6\\Shaders");
	FySetModelPath("Data\\NTU6\\Scenes");
	FySetTexturePath("Data\\NTU6\\Scenes\\Textures");
	FySetScenePath("Data\\NTU6\\Scenes");

	// create a viewport
	vID = FyCreateViewport(0, 0, 1024, 768);
	FnViewport vp;
	vp.ID(vID);

	// create a 3D scene
	sID = FyCreateScene(10);
	FnScene scene;
	scene.ID(sID);

	// load the scene
	scene.Load("gameScene02");
	scene.SetAmbientLights(1.0f, 1.0f, 1.0f, 0.6f, 0.6f, 0.6f);

	// load the terrain
	tID = scene.CreateObject(OBJECT);
	FnObject terrain;
	terrain.ID(tID);
	BOOL beOK1 = terrain.Load("terrain");
	terrain.Show(FALSE);

	// set terrain environment
	terrainRoomID = scene.CreateRoom(SIMPLE_ROOM, 10);
	FnRoom room;
	room.ID(terrainRoomID);
	room.AddObject(tID);

	// load the character
	FySetModelPath("Data\\NTU6\\Characters");
	FySetTexturePath("Data\\NTU6\\Characters");
	FySetCharacterPath("Data\\NTU6\\Characters");
	actorID = scene.LoadCharacter("Lyubu2");
	donzoID = scene.LoadCharacter("Donzo2");
	robberID = scene.LoadCharacter("Robber02");

	// put the character on terrain
	float pos[3], fDir[3], uDir[3], pos2[3], fDir2[3], uDir2[3], pos3[3], fDir3[3], uDir3[3];
	FnCharacter actor, donzo, robber;
	actor.ID(actorID);
	donzo.ID(donzoID);
	robber.ID(robberID);
	pos[0] = 3569.0f; pos[1] = -3208.0f; pos[2] = 1000.0f;
	fDir[0] = -0.983f; fDir[1] = -0.143f; fDir[2] = 0.0f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	pos2[0] = 3500.0f; pos2[1] = -3150.0f; pos2[2] = 1000.0f;
	fDir2[0] = -0.983f; fDir2[1] = -0.143f; fDir2[2] = 0.0f;
	uDir2[0] = 0.0f; uDir2[1] = 0.0f; uDir2[2] = 1.0f;
	pos3[0] = 3600.0f; pos3[1] = -3250.0f; pos3[2] = 1000.0f;
	fDir3[0] = -0.983f; fDir3[1] = -0.143f; fDir3[2] = 0.0f;
	uDir3[0] = 0.0f; uDir3[1] = 0.0f; uDir3[2] = 1.0f;
	actor.SetDirection(fDir, uDir);
	donzo.SetDirection(fDir2, uDir2);
	robber.SetDirection(fDir3, uDir3);

	actor.SetTerrainRoom(terrainRoomID, 10.0f);
	donzo.SetTerrainRoom(terrainRoomID, 10.0f);
	robber.SetTerrainRoom(terrainRoomID, 10.0f);
	beOK = actor.PutOnTerrain(pos);
	beOK = donzo.PutOnTerrain(pos2);
	beOK = robber.PutOnTerrain(pos3);

	// Get two character actions pre-defined at Lyubu2
	idleID = actor.GetBodyAction(NULL, "Idle");
	runID = actor.GetBodyAction(NULL, "Run");
	walkID = actor.GetBodyAction(NULL, "Walk");
	DieID = actor.GetBodyAction(NULL, "Die");
	AttactID = actor.GetBodyAction(NULL, "NormalAttack1");

	idle2ID = donzo.GetBodyAction(NULL, "Idle");
	run2ID = donzo.GetBodyAction(NULL, "Run");
	walk2ID = donzo.GetBodyAction(NULL, "Walk");
	Die2ID = donzo.GetBodyAction(NULL, "Die");
	DamageLID = donzo.GetBodyAction(NULL, "DamageL");

	idle3ID = robber.GetBodyAction(NULL, "Idle");
	run3ID = robber.GetBodyAction(NULL, "Run");
	walk3ID = robber.GetBodyAction(NULL, "Walk");
	Die3ID = robber.GetBodyAction(NULL, "Die");
	DamageID = robber.GetBodyAction(NULL, "Damage1");

	// set the character to idle action
	curPoseID = idleID;
	curPose2ID = idle2ID;
	curPose3ID = Die3ID;
	actor.SetCurrentAction(NULL, 0, curPoseID);
	actor.Play(START, 0.0f, FALSE, TRUE);
	actor.TurnRight(90.0f);
	donzo.SetCurrentAction(NULL, 0, curPose2ID);
	donzo.Play(START, 0.0f, FALSE, TRUE);
	donzo.TurnRight(90.0f);
	robber.SetCurrentAction(NULL, 0, curPose3ID);
	robber.Play(START, 0.0f, FALSE, TRUE);
	robber.TurnRight(90.0f);

	// translate the camera
	cID = scene.CreateObject(CAMERA);
	FnCamera camera;
	camera.ID(cID);
	camera.SetNearPlane(5.0f);
	camera.SetFarPlane(100000.0f);

	// set camera initial position and orientation
	pos[0] = 4300.783f; pos[1] = -3200.686f; pos[2] = 93.046f;
	fDir[0] = -0.983f; fDir[1] = -0.143f; fDir[2] = -0.119f;
	uDir[0] = 0.0f; uDir[1] = 0.0f; uDir[2] = 1.0f;
	camera.SetPosition(pos);
	camera.SetDirection(fDir, uDir);

	//direction
	float pos_actor[3];
	actor.GetPosition(pos_actor);
	donzo.GetPosition(pos_actor);
	robber.GetPosition(pos_actor);

	float pos_camera[3];
	camera.GetPosition(pos_camera);

	float vector[3], vector_length;
	constant_distant = sqrt(pow(pos_actor[0] - pos_camera[0], 2) + pow(pos_actor[1] - pos_camera[1], 2));

	float mainLightPos[3] = { -4579.0, -714.0, 15530.0 };
	float mainLightFDir[3] = { 0.276, 0.0, -0.961 };
	float mainLightUDir[3] = { 0.961, 0.026, 0.276 };

	FnLight lgt;
	lgt.ID(scene.CreateObject(LIGHT));
	lgt.Translate(mainLightPos[0], mainLightPos[1], mainLightPos[2], REPLACE);
	lgt.SetDirection(mainLightFDir, mainLightUDir);
	lgt.SetLightType(PARALLEL_LIGHT);
	lgt.SetColor(1.0f, 1.0f, 1.0f);
	lgt.SetName("MainLight");
	lgt.SetIntensity(0.4f);

	// create a text object for displaying messages on screen
	textID = FyCreateText("Trebuchet MS", 18, FALSE, FALSE);

	// set Hotkeys
	FyDefineHotKey(FY_ESCAPE, QuitGame, FALSE);  // escape for quiting the game
	FyDefineHotKey(FY_UP, Movement, FALSE);      // Up for moving forward
	FyDefineHotKey(FY_RIGHT, Movement, FALSE);   // Right for turning right
	FyDefineHotKey(FY_LEFT, Movement, FALSE);    // Left for turning left
	FyDefineHotKey(FY_DOWN, Movement, FALSE);    // down for turning back

	// set Hotkeys2
	FyDefineHotKey(FY_W, Movement2, FALSE);      // Up for moving forward
	FyDefineHotKey(FY_A, Movement2, FALSE);   // Right for turning right
	FyDefineHotKey(FY_D, Movement2, FALSE);    // Left for turning left
	FyDefineHotKey(FY_S, Movement2, FALSE);    // down for turning back

	// set Hotkeys attact
	FyDefineHotKey(FY_SPACE, Attact, FALSE);      // Up for moving forward


	// define some mouse functions
	FyBindMouseFunction(LEFT_MOUSE, InitPivot, PivotCam, NULL, NULL);
	FyBindMouseFunction(MIDDLE_MOUSE, InitZoom, ZoomCam, NULL, NULL);
	FyBindMouseFunction(RIGHT_MOUSE, InitMove, MoveCam, NULL, NULL);

	// bind timers, frame rate = 30 fps
	FyBindTimer(0, 30.0f, GameAI, TRUE);
	FyBindTimer(1, 30.0f, RenderIt, TRUE);

	// invoke the system
	FyInvokeFly(TRUE);

}


/*-------------------------------------------------------------
30fps timer callback in fixed frame rate for major game loop
--------------------------------------------------------------*/
void GameAI(int skip)
{
	FnCharacter actor, donzo, robber;
	FnObject camera;
	camera.ID(cID);

	// play character pose
	actor.ID(actorID);
	actor.Play(LOOP, (float)skip, FALSE, TRUE);
	donzo.ID(donzoID);
	donzo.Play(LOOP, (float)skip, FALSE, TRUE);
	robber.ID(robberID);
	robber.Play(LOOP, (float)skip, FALSE, TRUE);

	//direction
	float pos_actor[3], actor_fDir[3], actor_uDir[3];
	actor.GetPosition(pos_actor);
	donzo.GetPosition(pos_actor);
	robber.GetPosition(pos_actor);

	float pos_camera[3], camera_fDir[3], camera_uDir[3];
	camera.GetPosition(pos_camera);
	actor.GetDirection(actor_fDir, actor_uDir);
	donzo.GetDirection(actor_fDir, actor_uDir);
	robber.GetDirection(actor_fDir, actor_uDir);

	float unit = sqrt(pow(actor_fDir[0], 2) + pow(actor_fDir[1], 2));
	float dist = 6.0f;

	//terrain
	FnObject terrain;
	terrain.ID(tID);

	float dir[3];
	dir[0] = 0.0f;
	dir[1] = 0.0f;
	dir[2] = -1.0f;

	float act_cam_ray[3];
	act_cam_ray[0] = camera_fDir[0];
	act_cam_ray[1] = camera_fDir[1];
	act_cam_ray[2] = camera_fDir[2];


	if (FyCheckHotKeyStatus(FY_DOWN) || FyCheckHotKeyStatus(FY_S))
	{
		actor.GetDirection(actor_fDir, actor_uDir);
		camera.GetDirection(camera_fDir, camera_uDir);
		if (fabs(actor_fDir[0] + camera_fDir[0]) < 0.1 && fabs(actor_fDir[1] + camera_fDir[1]) < 0.1) {
			actor.MoveForward(dist, TRUE, FALSE, 0.0f, TRUE);
			if (terrain.HitTest(pos_camera, dir) > 0) {
				pos_camera[0] = pos_actor[0] + (constant_distant / unit) * (-camera_fDir[0]);
				pos_camera[1] = pos_actor[1] + (constant_distant / unit) * (-camera_fDir[1]);
				camera.SetPosition(pos_camera);
			}
			else {
				// the camera is not on the terrain
				float dot = actor_fDir[0] * camera_fDir[0] + actor_fDir[1] * camera_fDir[1];
				if (dot > 0.5) {
					if (pos_camera[2] > 100) {
						pos_camera[2] -= 1.0f;
					}
					camera_fDir[2] += 0.05f;
				}
				else if (dot < -0.5) {
					if (pos_camera[2] < 140) {
						pos_camera[2] += 1.0f;
					}
					camera_fDir[2] -= 0.05f;
				}
				camera.SetPosition(pos_camera);
				camera.SetDirection(camera_fDir, camera_uDir);
			}
		}
		else if ((judge_dir >= 0 && judge_dir <= 40) || (judge_dir >= -69 && judge_dir <= -36)) {
			actor.TurnRight(5.0f);
			judge_dir++;
		}
		else {
			actor.TurnRight(-5.0f);
			judge_dir--;
		}
	}

	if (FyCheckHotKeyStatus(FY_UP) || FyCheckHotKeyStatus(FY_W))
	{
		actor.GetDirection(actor_fDir, actor_uDir);
		camera.GetDirection(camera_fDir, camera_uDir);

		if (fabs(actor_fDir[0] - camera_fDir[0]) < 0.1 && fabs(actor_fDir[1] - camera_fDir[1]) < 0.1) {
			actor.MoveForward(dist, TRUE, FALSE, 0.0f, TRUE);
			actor.GetPosition(pos_actor);
			camera.GetPosition(pos_camera);
			actor.GetDirection(actor_fDir, actor_uDir);
			camera.SetDirection(actor_fDir, camera_uDir);

			if (terrain.HitTest(pos_camera, dir) > 0) {
				/*	adjust = 2.0f;*/
				pos_camera[0] = pos_actor[0] + (constant_distant / unit) * (-actor_fDir[0]);
				pos_camera[1] = pos_actor[1] + (constant_distant / unit) * (-actor_fDir[1]);
				camera.SetPosition(pos_camera);

				float vector[3], vector_length;
				vector_length = sqrt(pow(pos_actor[0] - pos_camera[0], 2) + pow(pos_actor[1] - pos_camera[1], 2) + pow(pos_actor[2] - pos_camera[2], 2));
				vector[0] = (pos_actor[0] - pos_camera[0]) / vector_length;
				vector[1] = (pos_actor[1] - pos_camera[1]) / vector_length;
				vector[2] = -0.12f;
				camera.SetDirection(vector, camera_uDir);
			}

			else {
				// the camera is not on the terrain
				/*			if (terrain.HitTest(pos_camera, act_cam_ray) < 0){
				adjust *= -2.0f;
				actor.TurnRight(adjust);
				act_cam_ray[0] = pos_actor[0] - pos_camera[0];
				act_cam_ray[1] = pos_actor[1] - pos_camera[1];
				act_cam_ray[2] = pos_actor[2] - pos_camera[2];
				}*/

				float dot = actor_fDir[0] * camera_fDir[0] + actor_fDir[1] * camera_fDir[1];
				if (dot > 0.5) {
					if (pos_camera[2] > 100) {
						pos_camera[2] -= 1.0f;
					}
					camera_fDir[2] += 0.05f;
				}
				else if (dot < -0.5) {
					if (pos_camera[2] < 140) {
						pos_camera[2] += 1.0f;
					}
					camera_fDir[2] -= 0.05f;
				}
				camera.SetPosition(pos_camera);
				camera.SetDirection(camera_fDir, camera_uDir);
			}

		}
		else {
			actor.TurnRight(-5.0f);
			judge_dir--;

		}

	}

	if (FyCheckHotKeyStatus(FY_LEFT) || FyCheckHotKeyStatus(FY_A))
	{
		actor.GetDirection(actor_fDir, actor_uDir);
		camera.GetDirection(camera_fDir, camera_uDir);

		if (fabs(actor_fDir[0] - camera_fDir[0]) < 0.1 && fabs(actor_fDir[1] - camera_fDir[1]) < 0.1) {


			actor.TurnRight(-5.0f);
			judge_dir = 0;
			actor.GetPosition(pos_actor);
			camera.GetPosition(pos_camera);
			actor.GetDirection(actor_fDir, actor_uDir);

			camera.SetDirection(actor_fDir, camera_uDir);
			pos_camera[0] = pos_actor[0] + (constant_distant / unit) * (-actor_fDir[0]);
			pos_camera[1] = pos_actor[1] + (constant_distant / unit) * (-actor_fDir[1]);
			camera.SetPosition(pos_camera);

			float vector[3], vector_length;
			vector_length = sqrt(pow(pos_actor[0] - pos_camera[0], 2) + pow(pos_actor[1] - pos_camera[1], 2) + pow(pos_actor[2] - pos_camera[2], 2));
			vector[0] = (pos_actor[0] - pos_camera[0]) / vector_length;
			vector[1] = (pos_actor[1] - pos_camera[1]) / vector_length;
			vector[2] = -0.12f;
			camera.SetDirection(vector, camera_uDir);


		}
		else {
			actor.TurnRight(-5.0f);
			judge_dir--;
		}

	}

	if (FyCheckHotKeyStatus(FY_RIGHT) || FyCheckHotKeyStatus(FY_D))
	{
		actor.GetDirection(actor_fDir, actor_uDir);
		camera.GetDirection(camera_fDir, camera_uDir);

		if (fabs(actor_fDir[0] - camera_fDir[0]) < 0.1 && fabs(actor_fDir[1] - camera_fDir[1]) < 0.1) {

			actor.TurnRight(5.0f);
			judge_dir = 0;
			actor.GetPosition(pos_actor);
			camera.GetPosition(pos_camera);
			actor.GetDirection(actor_fDir, actor_uDir);

			camera.SetDirection(actor_fDir, camera_uDir);
			pos_camera[0] = pos_actor[0] + (constant_distant / unit) * (-actor_fDir[0]);
			pos_camera[1] = pos_actor[1] + (constant_distant / unit) * (-actor_fDir[1]);
			camera.SetPosition(pos_camera);

			float vector[3], vector_length;
			vector_length = sqrt(pow(pos_actor[0] - pos_camera[0], 2) + pow(pos_actor[1] - pos_camera[1], 2) + pow(pos_actor[2] - pos_camera[2], 2));
			vector[0] = (pos_actor[0] - pos_camera[0]) / vector_length;
			vector[1] = (pos_actor[1] - pos_camera[1]) / vector_length;
			vector[2] = -0.12f;
			camera.SetDirection(vector, camera_uDir);

		}
		else {
			actor.TurnRight(5.0f);
			judge_dir++;
		}

	}


}


/*----------------------
perform the rendering
C.Wang 0720, 2006
-----------------------*/
void RenderIt(int skip)
{
	FnViewport vp;

	// render the whole scene
	vp.ID(vID);
	vp.Render3D(cID, TRUE, TRUE);

	// get camera's data
	FnCamera camera;
	camera.ID(cID);

	float pos[3], fDir[3], uDir[3];
	camera.GetPosition(pos);
	camera.GetDirection(fDir, uDir);

	// show frame rate
	static char string[128];
	if (frame == 0) {
		FyTimerReset(0);
	}

	if (frame / 10 * 10 == frame) {
		float curTime;

		curTime = FyTimerCheckTime(0);
		sprintf(string, "Fps: %6.2f", frame / curTime);
	}

	frame += skip;
	if (frame >= 1000) {
		frame = 0;
	}



	FnCharacter actor, donzo, robber;
	actor.ID(actorID);
	donzo.ID(donzoID);
	robber.ID(robberID);

	float pos_actor[3], pos_donzo[3], pos_robber[3];
	actor.GetPosition(pos_actor);
	donzo.GetPosition(pos_donzo);
	robber.GetPosition(pos_robber);

	float dist_donz = sqrt(pow(pos_actor[0] - pos_donzo[0], 2) + pow(pos_actor[1] - pos_donzo[1], 2));
	float dist_robber = sqrt(pow(pos_actor[0] - pos_robber[0], 2) + pow(pos_robber[1] - pos_robber[1], 2));



	FnText text;
	text.ID(textID);

	text.Begin(vID);
	text.Write(string, 20, 20, 255, 0, 0);

	char posS[256], fDirS[256], uDirS[256], db[256], dist_donzS[256];
	sprintf(posS, "pos: %8.3f %8.3f %8.3f", pos[0], pos[1], pos[2]);
	sprintf(fDirS, "facing: %8.3f %8.3f %8.3f", fDir[0], fDir[1], fDir[2]);
	sprintf(uDirS, "up: %8.3f %8.3f %8.3f", uDir[0], uDir[1], uDir[2]);
	sprintf(db, "Donzo Blood: %8.3f ", donzoblood);
	sprintf(dist_donzS, "Donzo Dist: %8.3f ", dist_donz);

	text.Write(posS, 20, 35, 255, 255, 0);
	text.Write(fDirS, 20, 50, 255, 255, 0);
	text.Write(uDirS, 20, 65, 255, 255, 0);
	text.Write(db, 20, 80, 255, 255, 0);
	text.Write(dist_donzS, 20, 95, 255, 255, 0);

	text.End();

	// swap buffer
	FySwapBuffers();
}


/*------------------
movement control
-------------------*/
void Movement(BYTE code, BOOL4 value)
{
	FnCharacter actor;
	actor.ID(actorID);

	if (!value)
	{
		if (code == FY_LEFT || code == FY_RIGHT)
		{
			if (curPoseID == runID || curPoseID == walkID)
			{
				if (!FyCheckHotKeyStatus(FY_UP) && !FyCheckHotKeyStatus(FY_LEFT) && !FyCheckHotKeyStatus(FY_RIGHT) && !FyCheckHotKeyStatus(FY_DOWN))
				{
					curPoseID = idleID;
					actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
					actor.Play(START, 0.0f, FALSE, TRUE);
				}
			}
		}
		else
		{
			if (curPoseID == runID)
			{
				curPoseID = walkID;
				actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
				actor.Play(START, 0.0f, FALSE, TRUE);
				if (!FyCheckHotKeyStatus(FY_LEFT) && !FyCheckHotKeyStatus(FY_RIGHT) && !FyCheckHotKeyStatus(FY_DOWN))
				{
					curPoseID = idleID;
					actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
					actor.Play(START, 0.0f, FALSE, TRUE);
				}
			}
		}
	}

	else
	{
		if (code == FY_UP)
		{
			if (curPoseID == idleID || curPoseID == walkID)
			{
				curPoseID = runID;
				actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
				actor.Play(START, 0.0f, FALSE, TRUE);
			}
		}

		else
		{
			if (curPoseID == idleID)
			{
				curPoseID = walkID;
				actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
				actor.Play(START, 0.0f, FALSE, TRUE);
			}
		}

	}
}

/*------------------
movement2 control
-------------------*/
void Movement2(BYTE code, BOOL4 value)
{
	FnCharacter actor;
	actor.ID(actorID);

	if (!value)
	{
		if (code == FY_A || code == FY_D || code == FY_S)
		{
			if (curPoseID == runID || curPoseID == walkID)
			{
				if (!FyCheckHotKeyStatus(FY_W) && !FyCheckHotKeyStatus(FY_A) && !FyCheckHotKeyStatus(FY_D) && !FyCheckHotKeyStatus(FY_S))
				{
					curPoseID = idleID;
					actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
					actor.Play(START, 0.0f, FALSE, TRUE);
				}
			}
		}
		else
		{
			if (curPoseID == runID)
			{
				curPoseID = walkID;
				actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
				actor.Play(START, 0.0f, FALSE, TRUE);
				if (!FyCheckHotKeyStatus(FY_A) && !FyCheckHotKeyStatus(FY_D) && !FyCheckHotKeyStatus(FY_S))
				{
					curPoseID = idleID;
					actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
					actor.Play(START, 0.0f, FALSE, TRUE);
				}
			}
		}
	}

	else
	{
		if (code == FY_W)
		{
			if (curPoseID == idleID || curPoseID == walkID)
			{
				curPoseID = runID;
				actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
				actor.Play(START, 0.0f, FALSE, TRUE);
			}
		}

		else
		{
			if (curPoseID == idleID)
			{
				curPoseID = walkID;
				actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
				actor.Play(START, 0.0f, FALSE, TRUE);
			}
		}

	}
}


void Attact(BYTE code, BOOL4 value)
{
	FnCharacter actor, donzo, robber;
	actor.ID(actorID);
	donzo.ID(donzoID);
	robber.ID(robberID);

	float pos_actor[3], pos_donzo[3], pos_robber[3];
	actor.GetPosition(pos_actor);
	donzo.GetPosition(pos_donzo);
	robber.GetPosition(pos_robber);

	float dist_donz = sqrt(pow(pos_actor[0] - pos_donzo[0], 2) + pow(pos_actor[1] - pos_donzo[1], 2));
	float dist_robber = sqrt(pow(pos_actor[0] - pos_robber[0], 2) + pow(pos_robber[1] - pos_robber[1], 2));

	if (FyCheckHotKeyStatus(FY_SPACE)) {
		if (curPoseID == idleID) {

			curPoseID = AttactID;
			actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
			actor.Play(START, 0.0f, FALSE, TRUE);

			//attact sucessful (close enough)
			if (dist_donz < 50.0f) {
				if (donzoblood >= 0) {
					donzoblood = AttactSys(donzoblood, 1);

					curPose2ID = DamageLID;
					donzo.SetCurrentAction(0, NULL, curPose2ID, 5.0f);
					donzo.Play(ONCE, 0.0f, FALSE, TRUE);
					curPose2ID = idle2ID;
					donzo.SetCurrentAction(0, NULL, curPose2ID, 10.0f);
					donzo.Play(START, 0.0f, FALSE, TRUE);

				}
				else if (donzoblood < 0) {
					curPose2ID = Die2ID;
					donzo.SetCurrentAction(0, NULL, curPose2ID, 5.0f);
					donzo.Play(ONCE, 0.0f, FALSE, TRUE);
					curPose2ID = idle2ID;
					donzo.SetCurrentAction(0, NULL, curPose2ID, 250.0f);
					donzo.Play(START, 0.0f, FALSE, TRUE);
				}

			}
			else if (dist_robber < 50.0f) {
				robberblood = AttactSys(robberblood, 1);
				if (robberblood >= 0) {
					robberblood = AttactSys(robberblood, 1);
					curPose3ID = DamageID;
					robber.SetCurrentAction(0, NULL, curPose3ID, 5.0f);
					robber.Play(ONCE, 0.0f, FALSE, TRUE);

					curPose3ID = idle3ID;
					robber.SetCurrentAction(0, NULL, curPose3ID, 10.0f);
					robber.Play(START, 0.0f, FALSE, TRUE);

				}
				else if (robberblood < 0) {
					curPose3ID = Die3ID;
					robber.SetCurrentAction(0, NULL, curPose3ID, 5.0f);
					robber.Play(ONCE, 0.0f, FALSE, TRUE);

					curPose3ID = idle3ID;
					robber.SetCurrentAction(0, NULL, curPose3ID, 10.0f);
					robber.Play(START, 0.0f, FALSE, TRUE);
				}
			}

		}
	}
	else {
		curPoseID = idleID;
		actor.SetCurrentAction(0, NULL, curPoseID, 5.0f);
		actor.Play(START, 0.0f, FALSE, TRUE);

	}


}
float AttactSys(float blood, int attactType) {
	// live
	if (attactType == 1) {
		blood -= 5;
	}
	else {
		blood -= 10;
	}
	return blood;
}
/*------------------
quit the demo
C.Wang 0327, 2005
-------------------*/
void QuitGame(BYTE code, BOOL4 value)
{
	if (code == FY_ESCAPE) {
		if (value) {
			FyQuitFlyWin32();
		}
	}
}



/*-----------------------------------
initialize the pivot of the camera
C.Wang 0329, 2005
------------------------------------*/
void InitPivot(int x, int y)
{
	oldX = x;
	oldY = y;
	frame = 0;
}


/*------------------
pivot the camera
C.Wang 0329, 2005
-------------------*/
void PivotCam(int x, int y)
{
	FnObject model;

	if (x != oldX) {
		model.ID(cID);
		model.Rotate(Z_AXIS, (float)(x - oldX), GLOBAL);
		oldX = x;
	}

	if (y != oldY) {
		model.ID(cID);
		model.Rotate(X_AXIS, (float)(y - oldY), GLOBAL);
		oldY = y;
	}
}


/*----------------------------------
initialize the move of the camera
C.Wang 0329, 2005
-----------------------------------*/
void InitMove(int x, int y)
{
	oldXM = x;
	oldYM = y;
	frame = 0;
}


/*------------------
move the camera
C.Wang 0329, 2005
-------------------*/
void MoveCam(int x, int y)
{
	if (x != oldXM) {
		FnObject model;

		model.ID(cID);
		model.Translate((float)(x - oldXM) * 2.0f, 0.0f, 0.0f, LOCAL);
		oldXM = x;
	}
	if (y != oldYM) {
		FnObject model;

		model.ID(cID);
		model.Translate(0.0f, (float)(oldYM - y) * 2.0f, 0.0f, LOCAL);
		oldYM = y;
	}
}


/*----------------------------------
initialize the zoom of the camera
C.Wang 0329, 2005
-----------------------------------*/
void InitZoom(int x, int y)
{
	oldXMM = x;
	oldYMM = y;
	frame = 0;
}


/*------------------
zoom the camera
C.Wang 0329, 2005
-------------------*/
void ZoomCam(int x, int y)
{
	if (x != oldXMM || y != oldYMM) {
		FnObject model;

		model.ID(cID);
		model.Translate(0.0f, 0.0f, (float)(x - oldXMM) * 10.0f, LOCAL);
		oldXMM = x;
		oldYMM = y;
	}
}