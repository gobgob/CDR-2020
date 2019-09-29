from aiohttp import web
from compass import CompassWatcher

compass_watcher = CompassWatcher()


async def get_compass_value(request):
    return web.json_response(compass_watcher.get_value())


app = web.Application()
app.add_routes([web.get('/', get_compass_value)])

if __name__ == '__main__':
    web.run_app(app)
