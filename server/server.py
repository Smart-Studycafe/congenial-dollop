from flask import Flask, request
from flask_restx import Resource, Api
import pymysql
from utils import serializer, database_utils
from utils.validator import Validator
import sys

app = Flask(__name__)
api = Api(app)

@api.route('/health')
class Health(Resource):
    def get(self):
        return {"health": "ok"}

@api.route('/seats')
class Seats(Resource):
    def get(self):
        try:
            connection = database_utils.connect_db()
        except:
            return {"error": "Something went wrong."}, 500
        
        with connection:
            with connection.cursor(pymysql.cursors.DictCursor) as cursor:
                sql = "SELECT * FROM SEATS"
                cursor.execute(sql)
                result = cursor.fetchall()
                result = serializer.make_serializable_datetimes(result)
                
        return result
    
    def post(self):
        # body에 존재하는 데이터값을 가져옴
        
        # 현재 좌석 변동 (입장, 퇴장)
        # 1. 주어진 데이터 무결성 검사
        # 2. 유저 존재 유무 검사
        # 3. 트랜잭션 실행
        pass

@api.route('/register')
class Register(Resource):
    def get(self):
        # db에 연결부터 해보자.
        try:
            connection = database_utils.connect_db()
        except:
            return {"error": "Unable to connect DB."}, 500
        
        with connection:
            with connection.cursor(pymysql.cursors.DictCursor) as cursor:
                sql = "select * from user"
                cursor.execute(sql)
                result = cursor.fetchall()
        
        return result

    def put(self):
        # 유효성 검사 실시
        json_obj = request.get_json()
        # id 유효성 검사
        try:
            user_id = str(json_obj["user_id"])
            username = json_obj["username"]
            if not (Validator.validate_user_id(user_id) and Validator.validate_username(username)):
                print("Unable.")
                return "Invalid user_id and username."
        except:
            return "Something went wrong."
        
        # DB 연결
        try:
            connection = database_utils.connect_db()
        except:
            return {"error": "Unable to connect DB."}, 500
        
        with connection:
            with connection.cursor() as cursor:
                sql = f"insert into user values({user_id}, \"{username}\")"
                result = cursor.execute(sql)
                if result:
                    connection.commit()
                    result = {"result": "Transaction success."}
                else:
                    result = {"result": f"Transaction failed while inserting values {user_id}, \"{username}\""}

        return result

app.run(debug=True, port=8080)