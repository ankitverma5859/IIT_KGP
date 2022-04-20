/* Q1 */
SELECT 
	Name AS "tournament name", 
    TTYPE as "tournament type", 
    Surface AS "Surface Type",
    NumRounds AS "number of rounds"
FROM Tournament
ORDER BY NumRounds;

SELECT 
	Name AS "tournament name", 
    TTYPE as "tournament type", 
    Surface AS "Surface Type",
    NumRounds AS "number of rounds"
FROM Tournament
ORDER BY 4;
/* Q1 Ends */

/* Q2 */
SELECT Name
FROM Player
WHERE CCode = "AUS";
/* Q2 Ends */

/* Q3 */
SELECT 
    Name AS "tournament name",
    TTYPE AS "tournament type",
    STARTDATE AS "start date",
    ENDDATE AS "end date",
    NumRounds AS "number of rounds",
    DATEDIFF(STR_TO_DATE(REPLACE(ENDDATE, '-', ''), '%d %M %Y'), 
        STR_TO_DATE(REPLACE(STARTDATE, '-', ''), '%d %M %Y')) AS 'Duration'
FROM Tournament
WHERE
Surface = "Hard" AND NUMRounds > 5;
/* Q3 Ends */


/* Function to split string */
CREATE FUNCTION SPLIT_STR(   
    x VARCHAR(255),   
    delim VARCHAR(12),   
    pos INT 
) 
RETURNS VARCHAR(255) DETERMINISTIC 
RETURN REPLACE(SUBSTRING(SUBSTRING_INDEX(x, delim, pos),        
                        LENGTH(SUBSTRING_INDEX(x, delim, pos -1)) + 1),        
                        delim, ''); 
/* Function to split string ends here */

/* Q4 */
SELECT 
    Tournament.Name AS "tournament name",
    Tournament.TType AS "tournament type",
    SPLIT_STR(Tournament.StartDate, '-', 3) AS "corresponding year",
    COUNT(Tournament.TID) AS "total number of matches"
FROM Matches
INNER JOIN Tournament
ON Matches.TID = Tournament.TID
GROUP BY Tournament.TID;
/* Q4 Ends */


/* Q5 */
SELECT * 
FROM 
(
SELECT 
    Tournament.Name,
    Tournament.TType,
    DATEDIFF(STR_TO_DATE(REPLACE(ENDDATE, '-', ''), '%d %M %Y'), 
    STR_TO_DATE(REPLACE(STARTDATE, '-', ''), '%d %M %Y')) AS "NumberOfDays"
FROM Tournament
) as innertable
WHERE NumberOfDays > 12;
/* Q5 Ends */


/* Q6 */
SELECT DISTINCT(Player.Name)
FROM PlayedIn
INNER JOIN Registration
ON PlayedIn.RegistrNum = Registration.PID
INNER JOIN Player
ON Registration.PID = Player.PID 
WHERE PlayedIn.Seed IS NOT NULL;
/* Q6 Ends */

/* Q7 */
SELECT DISTINCT(Player.Name) 
FROM Player
INNER JOIN Registration
ON Player.PID = Registration.PID
INNER JOIN PLAYEDIN
ON Registration.RegistrNum = PlayedIn.RegistrNum
INNER JOIN Tournament
ON PlayedIN.TID = Tournament.TID
WHERE 
    Player.CCode = 'AUS' AND 
    Player.Gender = 'M' AND
    Tournament.Name = "French Open" AND 
    Tournament.TType = "Doubles" AND
    SPLIT_STR(Tournament.StartDate, '-', 3) = "2007";
/* Q7 Ends */

/* Q8 */
SELECT PLAYER.Name
FROM Player
INNER JOIN Registration
ON Player.PID = Registration.PID
INNER JOIN PlayedIn
ON Registration.RegistrNum = PlayedIn.RegistrNum
INNER JOIN Tournament
ON PlayedIn.TID = Tournament.TID
WHERE 
    Tournament.Name = "Wimbledon" AND
    Tournament.TType = "Doubles" AND
    SPLIT_STR(Tournament.StartDate, '-', 3) = "2007";
;
/* Q8 Ends */

/* Q9 */
SELECT DISTINCT(Player.Name)
FROM Player
INNER JOIN Registration
ON Player.PID = Registration.PID
INNER JOIN PlayedIn
ON Registration.RegistrNum = PlayedIn.RegistrNum
INNER JOIN Tournament
ON PlayedIn.TID = Tournament.TID
WHERE
    Tournament.TType = "Singles" AND
    Tournament.Name = "Australian Open" AND
    SPLIT_STR(Tournament.StartDate, '-', 3) = "2007"
; 
/* Q9 Ends */

/* Q10 */
SELECT DISTINCT(Player.Name)
FROM Player
INNER JOIN Registration
ON Player.PID = Registration.PID
INNER JOIN PlayedIn
ON Registration.RegistrNum = PlayedIn.RegistrNum
INNER JOIN Tournament
ON PlayedIn.TID = Tournament.TID
WHERE
    Tournament.Surface = "Clay" AND
    Tournament.TType = "Doubles"
/* Q10 Ends */

/* Q11 */
SELECT DISTINCT(Matches.MID), Player.Name AS "Winner" 
FROM Matches
INNER JOIN MatchResults
ON Matches.MID = MatchResults.MID
INNER JOIN Registration
ON MatchResults.Winner = Registration.RegistrNum
INNER JOIN Player
ON Registration.PID = Player.PID
INNER JOIN Tournament
On Matches.TID = Tournament.TID
WHERE 
    Tournament.Name = "US Open" AND
    SPLIT_STR(Tournament.StartDate, '-', 3) = "2007"
;

/* Q12 */
SELECT * FROM
(
SELECT Player.Name, Count(Player.Name)
FROM Player
INNER JOIN Registration
ON Player.PID = Registration.PID
INNER JOIN PlayedIn
ON Registration.RegistrNum = PlayedIn.RegistrNum
INNER JOIN Tournament
ON PlayedIn.TID = Tournament.TID
GROUP BY Player.Name
ORDER BY Count(Player.Name) DESC
) as innertable
LIMIT 1;
/**/

/* Q13 */
SELECT DISTINCT(partners.Name)
FROM Player p, Registration r, PlayedIn pi, Tournament t, 
(SELECT P1.PID, P2.Name
FROM 
    Player P1, Player P2,
    Registration R1, Registration R2,
    PlayedIn PI, Tournament T
WHERE
    P1.Pid != P2.PID AND
    P1.Pid = R1.PID AND R1.RegistrNum = PI.RegistrNum AND
    P2.Pid = R2.PID AND R2.RegistrNum = PI.RegistrNum AND
    PI.TID = T.TID AND
    T.TType = "Doubles"
) AS partners
WHERE
    p.PID = partners.PID AND p.PID = r.PID AND pi.RegistrNum = r.RegistrNum
    AND t.TID = pi.TID AND t.TType = "Singles" AND pi.seed IS NOT NULL
;
/* Q13 Ends */




