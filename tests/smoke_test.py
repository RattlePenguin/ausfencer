import json, pathlib, socket, sqlite3, subprocess, sys, tempfile, time, urllib.request, urllib.error
binary = pathlib.Path(sys.argv[1]).resolve()

def request(method, path, body=None, status=200):
    data = None if body is None else json.dumps(body).encode()
    req = urllib.request.Request('http://127.0.0.1:8080/api/'+path, data=data, method=method,
                                 headers={'Content-Type':'application/json'})
    try:
        response = urllib.request.urlopen(req, timeout=2)
    except urllib.error.HTTPError as error:
        response = error
    with response:
        assert response.status == status, (method, path, response.status, status)
        return json.load(response)

with socket.socket() as probe:
    probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    probe.bind(('127.0.0.1',8080))
with tempfile.TemporaryDirectory(prefix='ausfencer-http-') as scratch:
    cwd = pathlib.Path(scratch)
    def start():
        log = (cwd/'server.log').open('a')
        process = subprocess.Popen([binary], cwd=cwd, stdout=log, stderr=log)
        log.close()
        try:
            for _ in range(100):
                assert process.poll() is None, (cwd/'server.log').read_text()
                try:
                    request('GET','fencers')
                    return process
                except (OSError, urllib.error.URLError):
                    time.sleep(0.05)
            raise AssertionError('Server readiness timeout')
        except BaseException:
            process.terminate(); process.wait(timeout=5)
            raise
    def stop(process):
        process.terminate()
        process.wait(timeout=5)
    process = start()
    try:
        assert request('GET','fencers') == {'fencers':[]}
        left = request('POST','fencers',dict(first_name='Left',last_name='Fencer',birth_year=2000))['id']
        right = request('POST','fencers',dict(first_name='Right',last_name='Fencer',birth_year=2001))['id']
        bout = dict(left_fencer_id=left,right_fencer_id=right,timestamp=1700000000,weapon='foil',time=180,
                    left_score=15,right_score=12,left_yellow=1,right_yellow=0,left_red=0,right_red=1)
        created = request('POST','bouts',bout)
        item = 'bouts/'+str(created['id'])
        assert request('GET',item) == created
        assert request('GET',f'bouts?fencer_id={right}&page=1&limit=1')['bouts'] == [created]
        request('DELETE',f'fencers/{left}',status=409)
        request('POST','bouts',dict(bout,left_fencer_id=9999),409)
        request('POST','bouts',dict(bout,left_score=100),400)
        request('GET','bouts?page=2junk',status=400)
        updated = request('PUT',item,dict(bout,weapon='sabre',left_score=99))
        assert request('GET',item) == updated
        guest = request('POST','bouts',dict(bout,left_fencer_id=None,right_fencer_id=None,weapon='epee'))
        request('DELETE','bouts/'+str(guest['id']))
        request('GET','bouts/'+str(guest['id']),status=404)
        retained = request('POST','bouts',dict(bout,left_fencer_id=None))
    finally:
        stop(process)
    process = start()
    try:
        assert request('GET',item) == updated
        assert request('GET','bouts/'+str(retained['id'])) == retained
        assert len(request('GET','fencers')['fencers']) == 2
        assert len(request('GET','bouts')['bouts']) == 2
        request('DELETE',item)
        request('DELETE',f'fencers/{left}')
        connection = sqlite3.connect(cwd/'db.ausfencer')
        assert connection.execute('PRAGMA foreign_key_check').fetchall() == []
        connection.close()
    finally:
        stop(process)
    assert 'using 2 threads' in (cwd/'server.log').read_text()
with tempfile.TemporaryDirectory(prefix='ausfencer-startup-') as scratch:
    (pathlib.Path(scratch)/'db.ausfencer').mkdir()
    result = subprocess.run([binary],cwd=scratch,capture_output=True,text=True,timeout=5)
    assert result.returncode != 0 and result.stderr.strip(), result
print('PASS: live CRUD, filter, validation, guests, conflicts, restart persistence, no seeding, no orphans, 2 workers, startup failure')
