set ANSI_NULLS ON
set QUOTED_IDENTIFIER ON
GO
------------------------------------------------------------------------------------------------------------

ALTER   PROC [dbo].[spGetAccountInfo]
 @AID  int      
, @ServerID int = 0
AS    
BEGIN  
 SET NOCOUNT ON    

 SELECT AID, UserID, UGradeID, Name, HackingType
 , DATEPART(yy, EndHackingBlockTime) AS HackBlockYear, DATEPART(mm, EndHackingBlockTime) AS HackBlockMonth    
 , DATEPART(dd, EndHackingBlockTime) AS HackBlockDay, DATEPART(hh, EndHackingBlockTime) AS HackBlockHour    
 , DATEPART(mi, EndHackingBlockTime) AS HackBlockMin
 , DATEDIFF(mi, GETDATE(), EndHackingBlockTime) AS 'HackingBlockTimeRemainderMin'
 , IsPowerLevelingHacker
 , DATEDIFF(mi, PowerLevelingRegDate, GETDATE()) AS 'PowerLevelingRegPassedTimeMin'
 , VIPWalls, LastVIPWall
 FROM Account(NOLOCK) WHERE AID = @AID      

 update Account set LastLoginTime = getdate(), ServerID = @ServerID  where aid = @aid  
END


