# Lame

This is the loader and cooker (processor) for CONK.

Here's the procedure for using Lame.

1) Call InitLame(). This just initialises all the internal lists and stuff like that.
2) Call LoadGameConfig(GameFile). The game file is file with all the list of all files to load, one file per line. This than makes up an array (levelconfigarray) of pointers to the names of the files to load in. This routine will return a pointer to the start of the file name array, place this pointer into lamereq->lr_Files.
3) Call LoadFiles(lamereq). You must supply your own lamereq structure. The first part of this structure must be completed by you. This routine will return an error code. If this code is less than LAME_FAILAT, then don't worry about it.
4) Pull what ever data you need out of the lamereq structure and go for broke.
5) Call FreeFiles(). This Deallocates all the run time data, but actually leaves the files in memory.
 This is so that when we go to load a file, we can see if it is resident in memory already, and if so don't load it again. This infinitly speeds up the reloading of level files. There is a ton of memory handling done though, so that if the MyAllocVec() routine can't allocated the desired amount of memory, it will dump all the files that have nothing to do with this level (given by the FileNode->fn_UsedInLevel flag), and it will also dump loaded chunks that have been cooked (ChunkNode->cn_Cooked), and can be freed (ChunkNode->cn_Freeable). If a chunk out of a file  is freed then the files fn_Complete flag is set to False. This is to say that the file will need to be reloaded again.
6) For each level just keep jumping through steps 2-5.
7) Call KillLame(). This kills all the loaded files completely.
