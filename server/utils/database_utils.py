from dotenv import load_dotenv
from .message_maker import Message
from . import serializer
import pymysql
import os
import sys
load_dotenv()

class DatabaseConnector:
    def __init__(self):
        pass
    
    def connect(self):
        try:
            self.connection = pymysql.connect(
                host=os.getenv("DB_HOST"),
                database=os.getenv("DB_NAME"),
                user=os.getenv("DB_USERNAME"),
                password=os.getenv("DB_PASSWORD"),
                ssl_ca=os.getenv("SSL_CERT")
            )
            return self.connection
        except:
            return False
    
    def disconnect(self):
        self.connection.close()
    
class DatabaseQueryExecutor:
    def select_seats(conn):
        cursor = conn.cursor(pymysql.cursors.DictCursor)
        
        with conn:
            sql = "select * from seats"
            cursor.execute(sql)
            result = cursor.fetchall()
            result = serializer.make_serializable_datetimes(result)
        
        return result
    
    def insert_user(conn, user_id, username):
        cursor = conn.cursor()
        
        with conn.cursor() as cursor:
            sql = f"insert into user values({user_id}, \"{username}\")"
            result = cursor.execute(sql)
            if result:
                conn.commit()
                result = {"result": "Transaction success."}
                return Message.Success("Transaction success.")
            else:
                return Message.Success(f"Transaction failed while inserting values {user_id}, \"{username}\"")
    
    def select_users(conn):
        cursor = conn.cursor(pymysql.cursors.DictCursor)
        
        with conn:
            sql = "select * from user"
            cursor.execute(sql)
            result = cursor.fetchall()
        
        return result