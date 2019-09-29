from aiohttp import web
from compass import CompassWatcher


async def get_compass_value(request):
    compass_watcher = request.app['compass_watcher']

    if compass_watcher.error:
        return web.json_response(status=500, data={"message": compass_watcher.error})

    return web.json_response(compass_watcher.get_value())


async def start_background_task(app):
    app['compass_watcher'] = CompassWatcher()


async def cleanup_background_task(app):
    app['compass_watcher'].stop()


app = web.Application()
app.add_routes([web.get('/', get_compass_value)])
app.on_startup.append(start_background_task)
app.on_cleanup.append(cleanup_background_task)
web.run_app(app)
