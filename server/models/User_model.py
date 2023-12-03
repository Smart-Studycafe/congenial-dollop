from flask_restx import fields, Api

class User_model:
    def __init__(self, api:Api):
        self.model = api.model('user', strict=True, model={
            'user_id': fields.Integer(default=1, required=True),
            'username': fields.String(default="username", required=True),
        })
    
    def get_model(self):
        return self.model