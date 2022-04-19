Description of the Application:
This application downloads and parses data from https://www.worldometers.info/coronavirus/ website using lex/yacc. It help you run queries
to get the <field> value of a country/continent/world. Field values could be active cases, daily deaths, etc. 
The application is also provides feature to get the value of a particar <field> for a given time range and closest country for the same.

Dependencies:
	python 3.9.6 or above 

Python Packages:
	Request
	ply.lex
	ply.yacc
	datetime
	shutil
	re 

File Structure

/21CS60A04_CL2_A5
	main.py 									-> Task 2 -> Task 1 imported here as it is a pre-requisite
	download_webiste.py 						-> Task 1
	common.py
	constants.py
	covid_country_list.py
	covid_statistics.py
	CovidStats.py
	download_covid_wiki.py
	worldometers_countrylist.txt
	readme.txt
	/webpages
		/africa
		/asia
		/europe
		/north_america
		/south_america
		/ocenia
		index.html
		yesterday.html  		           			
	/logs
		logs_<TIMESTAMP>.txt
	/covid_wiki
	    /country
	    /responses
	    /timeline
	/resources
	/ply
		__init__.py
		cpp.py
		ctokens.py
		lex.py
		yacc.py
		ygen.py
** some of the directories are created during the execution of the program.


How to run the application:

Command:	python main.py

			
Follow the menu driven instructions to answer your queries.

BELOW INSTRUCTION IS FOR TIMELINE AND RESPONSE DATA ONLY
-- *** ---
IMPORTANT: DATE format should be followed very strictly for time range queries. Also it is case sensitive.
			Format:
					<Month> <Date> <Year>

					<Month> : Jan | Feb | Mar | Apr | May | Jun | Jul | Aug | Sept | Oct | Nov | Dec
					<Date>  : 01 - 31 (Note that 01 should be used instead of just '1', double digit format is a must)
					<Year>  : YYYY

			Example: Jan 01 2022
					 Jan 31 2022
					 Dec 20 2020

IF data for a particular field or date is not found, terminal will not show the results for the same.
IF user enters a wrong value during the menu driven execution, USER will be prompted with "Invalid Value" message.
-- *** ---

BELOW INSTRUCTION IS FOR WIKIPEDIA NEWS ITEMS
-- *** ---
ASSIGNMENT 2 TASK NO.           MAPPED NUMBER in MENU after adding the ASSIGNMENT 1
1.                                  4. Worldwide Wikipedia News (Shows Worldwide news and responses of a given timerange along with Wordcloud)
2.                                  5. World Cloud of common/covid words of non-overlapping time-range, percentage of covid and common words, top 20 for the given combinations
3.                                  6. Date Range of a given country for the available news
4.                                  6. News of a given country for a given time-range
5.                                  6. Top 3 countries on the basis of Jaccard Similarity for common
6.                                  6. Top 3 countries on the basis of Jaccard Similarity for covid words
-- *** ---