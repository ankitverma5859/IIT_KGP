show tables;

SELECT * 
FROM CountryCodes;

SELECT *
FROM Matches;

SELECT *
FROM MatchResults;

SELECt * 
FROM PlayedIn;

SELECT *
FROM Player;

SELECT * 
FROM Registration;

SELECT *
FROM RetiredMatch;

SELECT * 
FROM SetScore;

SELECT * FROM
Tiebreaker;

SELECT *
FROM Tournament;

SELECT TID, Name, Surface
FROM Tournament;

SELECT 
TID AS "Tournament ID", Name as "Tournamenent Name", Surface as "Floor Type"
FROM Tournament;

SELECT CONCAT(StartDate, ' to ', EndDate) AS "Tournament Date"
FROM Tournament;

SELECT * 
FROM Tournament
ORDER BY NumRounds DESC;

SELECT * 
FROM Tournament
ORDER BY NumRounds ASC;

SELECT *
FROM Tournament
ORDER BY 
NumRounds ASC,
StartDate DESC;

/*
	Using the length of a columns as a SORT expression
*/
SELECT 
	Name,
    LENGTH(Name) as len
FROM Tournament
ORDER BY len ASC;

/*
	Selecting Columns number as a sort parameter.
*/
SELECT 
	NumRounds,
    StartDate
FROM Tournament
ORDER BY
	1 ASC,
    2 DESC;
    
/*
	Selecting distinct values of a column
*/    
SELECT DISTINCT Location
FROM Tournament;
