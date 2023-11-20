from dotenv import load_dotenv
import pymysql
import os
load_dotenv()

def connect_db():
    connection = pymysql.connect(
        host=os.getenv("DB_HOST"),
        database=os.getenv("DB_NAME"),
        user=os.getenv("DB_USERNAME"),
        password=os.getenv("DB_PASSWORD"),
        ssl_ca=os.getenv("SSL_CERT")
    )

    return connection