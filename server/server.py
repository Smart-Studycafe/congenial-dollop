import sys
from flask import Flask, request
from flask_restx import Resource, Api, fields
from utils.Database_utils import DatabaseConnector, DatabaseQueryExecutor
from utils.Validator import Validator
from utils.Message_maker import Message
from utils.Certificator import Certificator
from models.Seats_model import Seats_model
from models.User_model import User_model

app = Flask(__name__)
api = Api(app)
CERTIFICATOR = Certificator()
SEATS_MODEL = Seats_model(api).get_model()
USER_MODEL = User_model(api).get_model()

# middleware that checks authorization with api_key
def check_authorization(request, certificator):
    args = request.args
    key = args.get("api_key")
    return certificator.check_available(key)
    

@api.route('/health')
class Health(Resource):
    def get(self):
        return Message.Success("Health is good.")

@api.route('/seats')
@api.doc(params={'api_key': 'An api key that noticed.'})
class Seats(Resource):
    # 좌석 정보 반환 API
    def get(self):
        if not check_authorization(request, CERTIFICATOR):
            return Message.Failure("API Key is not certificated.")
        
        conn = DatabaseConnector().connect()
        
        if not conn:
            return Message.FailureDbConnection()
        
        try:
            result = DatabaseQueryExecutor.select_seats(conn)
            # 트랜잭션 결과 비어있는 경우
            if not result:
                result = Message.Failure("Seats table is empty.")
        except:
            result = Message.Failure("Error occured while executing select transaction.")
            
        return result
    
    # 좌석 삽입 api
    @api.doc(body=SEATS_MODEL)
    def post(self):
        if not check_authorization(request, CERTIFICATOR):
            return Message.Failure("API Key is not certificated.")
        
        json_obj = request.get_json()
        
        try:
            # TODO: 유효성 검사 해야함
            seat_id = str(json_obj["seat_id"])
            user_id = str(json_obj["user_id"])
            usage_start = str(json_obj["usage_start"])
            usage_end = str(json_obj["usage_end"])
            Validator.validate_seat(seat_id, user_id, usage_start, usage_end)
        except:
            return Message.Failure("Invalid request body."), 400
        
        conn = DatabaseConnector().connect()
        
        if not conn:
            return Message.FailureDbConnection()
        
        try:
            result = DatabaseQueryExecutor.insert_seats(conn, seat_id, user_id, usage_start, usage_end)
            if result:
                result =  Message.Success("Insert seat execution succeed.")
        except:
            result = Message.Failure("Error occured while executing insert transaction.")
        
        return result
    
    # 좌석 정보 변동
    @api.doc(body=SEATS_MODEL)
    def put(self):
        if not check_authorization(request, CERTIFICATOR):
            return Message.Failure("API Key is not certificated.")
        
        # 현재 좌석 변동 (입장, 퇴장)
        # 1. 주어진 데이터 무결성 검사
        # 2. 유저 존재 유무 검사
        # 3. 트랜잭션 실행
        pass

@api.route('/register')
@api.doc(params={'api_key': 'An api key that noticed.'})
class Register(Resource):
    # 유저 정보 반환 Rest API
    def get(self):
        if not check_authorization(request, CERTIFICATOR):
            return Message.Failure("API Key is not certificated.")
        
        conn = DatabaseConnector().connect()
        if not conn:
            return Message.FailureDbConnection()
        
        try:
            result = DatabaseQueryExecutor.select_users(conn)
            if not result:
                result = Message.Failure("User table is empty.")
        except:
            result = Message.Failure("Error occured while executing select transaction.")
        
        return result

    # 새로운 유저 추가 Rest API
    def post(self):
        if not check_authorization(request, CERTIFICATOR):
            return Message.Failure("API Key is not certificated.")
        
        # 유효성 검사 실시
        json_obj = request.get_json()
        
        # id 유효성 검사
        try:
            user_id = str(json_obj["user_id"])
            username = json_obj["username"]
            Validator.validate_user(user_id, username)
        except:
            return Message.Failure("user_id and username is not valid or not in request body."), 400
        
        # DB 연결
        conn = DatabaseConnector().connect()
        
        if not conn:
            return Message.FailureDbConnection()
        
        try:
            result = DatabaseQueryExecutor.insert_user(conn, user_id, username)
            if result:
                result = Message.Success("Insert user excuted successfully.")
        except:
            result = Message.Failure("Error occured while executing insert transaction.")
        
        return result

if __name__ == "__main__":
    app.run(debug=True, port=8080)