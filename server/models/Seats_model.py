from flask_restx import fields, Api

class Seats_model:
    def __init__(self, api:Api):
        self.model = api.model('seats', strict=True, model={
            'seat_id': fields.Integer(default=1, required=True),
            'user_id': fields.Integer(default=1, required=True),
            'usage_start': fields.DateTime(required=True),
            'usage_end': fields.DateTime(required=True),
        })
    
    def get_model(self):
        return self.model