import covid_statistics


class CovidStats:

    def __init__(self, total_cases, new_cases, total_deaths, new_deaths, total_recovered, new_recovered, active_cases,
                 serious_cases, total_cases_per_million, deaths_per_million, total_tests, tests_per_million):
        self.total_cases = covid_statistics.extract_stat(total_cases)
        self.new_cases = covid_statistics.extract_stat(new_cases)
        self.total_deaths = covid_statistics.extract_stat(total_deaths)
        self.new_deaths = covid_statistics.extract_stat(new_deaths)
        self.total_recovered = covid_statistics.extract_stat(total_recovered)
        self.new_recovered = covid_statistics.extract_stat(new_recovered)
        self.active_cases = covid_statistics.extract_stat(active_cases)
        self.serious_cases = covid_statistics.extract_stat(serious_cases)
        self.total_cases_per_million = covid_statistics.extract_stat(total_cases_per_million)
        self.deaths_per_million = covid_statistics.extract_stat(deaths_per_million)
        self.total_tests = covid_statistics.extract_stat(total_tests)
        self.tests_per_million = covid_statistics.extract_stat(tests_per_million)

    def dict_stat_data(self):
        stat = {'total_cases': self.total_cases, 'new_cases': self.new_cases, 'total_deaths': self.total_deaths,
                'new_deaths': self.new_deaths, 'total_recovered': self.total_recovered,
                'new_recovered': self.new_recovered, 'active_cases': self.active_cases,
                'serious_cases': self.serious_cases, 'total_cases_per_million': self.total_cases_per_million,
                'deaths_per_million': self.deaths_per_million, 'total_tests': self.total_tests,
                'tests_per_million': self.tests_per_million}
        return stat