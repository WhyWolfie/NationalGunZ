set ANSI_NULLS ON
set QUOTED_IDENTIFIER ON
GO
ALTER proc [dbo].[spUpdateAccountLastLogoutTime]
 @AID int
, @VIPWalls int
, @LastVIPWall int
as
 set nocount on

 update Account set LastLogoutTime = getdate(),
 VIPWalls = @VIPWalls, LastVIPWall = @LastVIPWall where AID = @AID


