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

/*
	Filtering Data
*/
SELECT *
FROM Tournament
WHERE Name = "Australian Open";

SELECT * 
FROM Tournament
WHERE NumRounds = 5;

/*
	Filtering with AND OR Operators
*/
SELECT * 
FROM Tournament 
WHERE NumRounds = 5 AND Surface = "Hard";

SELECT * 
FROM Tournament 
WHERE NumRounds = 5 OR Surface = "Hard";

SELECT * 
FROM Tournament 
WHERE NumRounds = 5 OR Surface = "Hard"
ORDER BY NumRounds;


SELECT * 
FROM Tournament 
WHERE (NumRounds = 5 OR Surface = "Hard") AND TType = "Singles"
ORDER BY NumRounds;


SELECT *
FROM Tournament
WHERE NumRounds != 5;

SELECT *
FROM Tournament
WHERE NumRounds > 5;

SELECT *
FROM Tournament
WHERE NumRounds >= 5;

SELECT *
FROM Tournament
WHERE NumRounds < 5;

SELECT *
FROM Tournament
WHERE NumRounds <= 5;

/*
	Filtering on Date
*/
SELECT *
FROM Tournament
Where StartDate < '2009-1-1'
ORDER By StartDate;

SELECT *
FROM Tournament
Where StartDate > '2009-1-1'
ORDER By StartDate;
    
    
/*
	Filtering on Varchar 
*/
SELECT *
FROM Tournament 
WHERE Name != 'Wimbledon';

SELECT *
FROM Tournament 
WHERE Name < 'Australian Open';

SELECT *
FROM Tournament 
WHERE Name < 'Brasil Open';
