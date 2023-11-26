class Message:
    # 결과 message를 포함한 딕셔너리를 반환하는 클래스
    
    def Failure(message: str):
        return {"result": "failure", "message": message}
    
    def Success(message: str):
        return {"result": "success", "message": message}, 200
    
    def FailureDbConnection():
        return {"result": "failure", "message": "Connection with DB has failed."}, 500