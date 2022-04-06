import requests
import time
import pandas as pd
import pendulum
import datetime as dt

# Press the green button in the gutter to run the script.
if __name__ == '__main__':

    dataset_dir = 'Dataset'
    subreddit = '<SUBREDDIT_NAME>'
    start_year = 2017
    end_year = 2022
    limit = 100

    start_timestamp = int(dt.datetime(start_year, 1, 1).timestamp())
    print(f'Start Data: {pendulum.from_timestamp(start_timestamp).to_datetime_string()}')
    print(f'Start Timestamp: {start_timestamp}')

    end_timestamp = int(dt.datetime(end_year, 1, 1).timestamp())
    print(f'End Data: {pendulum.from_timestamp(end_timestamp).to_datetime_string()}')
    print(f'End Timestamp: {end_timestamp}')

    timestamp = start_timestamp
    count = 0
    while timestamp <= end_timestamp:
        # Calculating the initial timestamp
        start_tms = timestamp
        start_date = pendulum.from_timestamp(start_tms).to_datetime_string()
        start_day = start_date.split(' ')[0]

        # Calculating the next day timestamp
        timestamp = timestamp + (24*60*60)
        end_tms = timestamp
        end_date = pendulum.from_timestamp(end_tms).to_datetime_string()
        end_day = end_date.split(' ')[0]

        #Creating filename
        dir_path = dataset_dir + '/' + subreddit + '/'
        filename = dir_path + str(start_day) + '_' + str(end_day) + '.csv'

        df = pd.DataFrame()
        res = requests.get(
            f'https://api.pushshift.io/reddit/search/submission/?subreddit={subreddit}&before={end_tms}&after={start_tms}&sort=desc&sort_type=created_utc&limit={limit}')
        response_json = res.json()
        #print(response_json)
        print(res.status_code)
        num_of_posts = len(response_json['data'])
        for itr in range(num_of_posts):
            #print(response_json['data'][itr]['title'])
            post_data = response_json['data'][itr]
            df = df.append({
                'subreddit': post_data['subreddit'],
                'author': post_data['author'],
                'id': post_data['id'],
                'title': post_data['title'],
                'full_link': post_data['full_link'],
                'score': post_data['score'],
                #'upvote_ratio': post_data['upvote_ratio'],
                'upvote_ratio': None,
                'gender': 'male'
            }, ignore_index=True)
        df.to_csv(filename)
        print(f'Saving {filename}')
        count += 1

        if count == 30:
            print(f'Data collected till till {end_day}.\nTaking a nap for 2 minutes!')
            time.sleep(120)
            count = 0
