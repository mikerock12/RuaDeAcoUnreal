"""Gera definições C++ e replays de dados exportados pelo motor TypeScript.

Não implementa regras nem ajusta expectativas: golden_fixtures.json é a fonte.
Execute depois do teste parityExporter no repositório RuaDeAco.
"""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT.parent / 'RuaDeAco/docs/parity/golden_fixtures.json'
D = json.loads(SOURCE.read_text(encoding='utf-8'))
OUT = ROOT / 'Data/generated'
OUT.mkdir(parents=True, exist_ok=True)
(OUT / 'golden_fixtures.json').write_bytes(SOURCE.read_bytes())
IDS = ['guto-barba', 'rafa-mare', 'noir-reflexo', 'astro-riso', 'dante-sinal', 'leo-violeta']
ENUMS = ['GutoBarba', 'RafaMare', 'NoirReflexo', 'AstroRiso', 'DanteSinal', 'LeoVioleta']
STATES = ['idle','walkForward','walkBackward','jump','fall','landing','crouch','blockStanding','blockCrouching','lightAttack','heavyAttack','kickAttack','specialAttack','hitStun','knockdown','wakeUp','grabbedFront','grabbedLifted','thrown','frozen','victory','knockout','dizzy']
DIRS = ['neutral', 'down', 'downForward', 'downBack', 'forward', 'back', 'up']
ACTIONS = D['inputActions']
def q(x): return json.dumps(x, ensure_ascii=True)
def n(x):
    if isinstance(x, bool): return str(x).lower()
    if isinstance(x, str): return q(x)
    return format(x, '.17g') + 'f' if isinstance(x, float) and not x.is_integer() else str(int(x))
def state(x): return 'FighterState::' + x[0].upper() + x[1:]
def rect(x): return 'Rect{' + ','.join(n(x[k]) for k in ['x','y','width','height']) + '}'
def hit(x, r=None):
    r = r or {'from': 0, 'to': 0}
    return 'HitboxDef{' + ','.join([rect(x), 'HitLevel::'+x['level'].capitalize(), 'HitKind::'+x['kind'].capitalize(),n(r['from']),n(r['to'])] + [n(x.get(k,v)) for k,v in [('damage',0),('chipDamage',0),('hitStun',0),('blockStun',0),('hitStop',0),('priority',0),('knockbackX',0),('knockbackY',0),('knockdown',False),('crouchAvoidable',False),('airAvoidable',False),('freezeFrames',0),('id',''),('offensiveDebuffFrames',0),('offensiveDebuffMultiplier',1)]]) + '}'
lines = ['// Gerado de golden_fixtures.json; não editar.', '// SHA256 '+hashlib.sha256(SOURCE.read_bytes()).hexdigest(), 'void EnrichCanonicalDefinition(FighterDef& d) {', 'switch(d.id) {']
for fid,en in zip(IDS,ENUMS):
    f = next(f for f in D['fighters'] if f['id']==fid)
    lines += ['case FighterId::'+en+': {']
    for k in ['maxHealth','walkSpeed','backwardSpeed','jumpSpeed','jumpForwardSpeed','jumpBackwardSpeed','gravity','weight']:
        lines += ['d.stats.'+k+' = '+n(f['stats'][k])+';']
    for i,p in enumerate(['standing','crouching','airborne','landing']):
        pose = D['collisionProfiles'][fid]['poses'][p]
        lines += [f'd.poseHurtboxes[{i}] = {{'+','.join(rect(b) for b in pose['hurtboxes'])+'};', f'd.posePushboxes[{i}] = '+rect(pose['pushbox'])+';']
    passive = f.get('passive', {})
    for k,key,default in [('passiveThreshold','damageThreshold',0),('passiveDuration','durationFrames',0),('passiveSpeed','speedMultiplier',1)]: lines += ['d.'+k+' = '+n(passive.get(key,default))+';']
    reaches=[max(0,box['x']+box['width']) for move in f['moves'].values() for phase in move['hitboxes'] for box in phase['boxes']]
    lines += ['d.cpuPreferredRange = '+n(max([62]+reaches)+18)+';']
    projectiles = list(f.get('projectiles',{}).values())
    for i,p in enumerate(projectiles):
        lines += [f'd.projectiles[{i}] = ProjectileDef{{'+','.join([q(p['id'])]+[n(p.get(k,v)) for k,v in [('offsetX',0),('offsetY',0),('velocityX',0),('lifeFrames',0),('armingFrames',0)]]+[n(p.get('spawnMode')=='targetSnapshot'),hit(p['hitbox']),n(p.get('maxActivePerOwner',-1))])+'};']
    lines += [f'd.projectileCount = {len(projectiles)};', 'd.canonicalMoves.clear();']
    for m in f['moves'].values():
        cmd = m['command']
        anim = m['animation'][0].upper()+m['animation'][1:]
        anim = {'JumpLightNeutral':'AirLight','JumpLightForward':'AirLight','JumpLightBackward':'AirLight','JumpHeavyNeutral':'AirHeavy','JumpHeavyForward':'AirHeavy','JumpHeavyBackward':'AirHeavy'}.get(anim,anim)
        if anim not in ['StandingLight','StandingHeavy','ForwardLight','ForwardHeavy','CrouchLight','CrouchHeavy','AirLight','AirHeavy','UniversalGrab']: anim = 'Super' if m.get('isSuper') else ('Special1' if m['state']=='specialAttack' else 'StandingHeavy')
        lines += ['{ MoveDef m{};', 'm.id = '+q(m['id'])+';', 'm.label = '+q(m['label'])+';', 'm.state = '+state(m['state'])+';', 'm.anim = AnimId::'+anim+';']
        for k,v in [('air',False),('isSuper',False),('meterCost',0),('totalFrames',0),('meterGainOnHit',0),('meterGainOnBlock',0),('lockFacing',False)]: lines += ['m.'+k+' = '+n(m.get(k,v))+';']
        lines += ['m.isGrab = '+n('grab' in m)+';', 'm.isSpecial = '+n(m['state']=='specialAttack')+';', 'm.crouch = '+n(m['animation'].startswith('crouch'))+';', 'm.trajectory = '+str(['neutral','forward','backward'].index(m['jumpTrajectory']) if 'jumpTrajectory' in m else -1)+';']
        lines += ['m.command = CommandDef{{'+','.join(str(DIRS.index(x)) for x in cmd.get('directions',[]))+'},'+','.join([str(sum(1<<ACTIONS.index(b) for b in cmd['buttons']))]+[n(cmd.get(k,v)) for k,v in [('maxGapFrames',1),('bufferFrames',5),('priority',0),('simultaneous',False)]])+'};']
        lines += ['m.animationKey = '+q(m['animation'])+';']
        ranges=[phase['range'] for phase in m['hitboxes'] if phase['boxes']]
        cues=[ev['frame'] for ev in m.get('events',[]) if ev['type'] in ['spawnProjectile','grantBuff','grantDamageReduction','throw']]
        impact_from=min(r['from'] for r in ranges) if ranges else min(cues) if cues else max(0,m['totalFrames']-1)//2
        impact_to=max(r['to'] for r in ranges) if ranges else max(cues) if cues else impact_from
        lines += [f'm.visualImpactFrom = {impact_from};',f'm.visualImpactTo = {impact_to};']
        boxes = [hit(b,phase['range']) for phase in D['effectiveHitboxes'][fid][m['id']] for b in phase['boxes']]
        lines += ['m.hitboxes = {'+','.join(boxes)+'};']
        if boxes: lines += ['m.hitbox = m.hitboxes.front();']
        lines += ['m.movement = {'+','.join('MovementDef{'+','.join(n(x) for x in [a['range']['from'],a['range']['to'],a.get('velocityX',0),a.get('velocityY',0),'velocityY' in a])+'}' for a in m.get('movement',[]))+'};']
        evs=[]
        for ev in m.get('events',[]):
            typ=['spawnProjectile','grantArmor','clearArmor','grantBuff','grantParry','grantDamageReduction','throw'].index(ev['type'])
            value = next((i for i,p in enumerate(projectiles) if p['id']==ev.get('projectileId')), -1) if typ==0 else ev.get('hits',ev.get('riposteDamage',0))
            evs.append('MoveEventDef{'+','.join(n(x) for x in [ev['frame'],typ,value,ev.get('durationFrames',0),ev.get('multiplier',1),ev.get('cooldownFrames',ev.get('durationFrames',0))])+'}')
        lines += ['m.events = {'+','.join(evs)+'};']
        lines += ['m.cancels = {'+','.join('CancelDef{'+','.join([str(c['range']['from']),str(c['range']['to']),str(['always','hit','block'].index(c['condition'])),'{'+','.join(q(x) for x in c['into'])+'}'])+'}' for c in m.get('cancels',[]))+'};']
        if 'grab' in m:
            g=m['grab']
            for k,v in [('holdStartFrame',14),('releaseFrame',46),('whiffRecoveryFrame',-1),('slam',False),('throwVelocityX',0),('throwVelocityY',0)]: lines += ['m.'+k+' = '+n(g.get(k,v))+';']
            if m['id']=='universalGrab':
                for j,vid in enumerate(IDS):
                    poses=D['universalGrabPoses'][fid+':'+vid]
                    lines += [f'm.grabPoses[{j}] = {{'+','.join('GrabPoseDef{'+','.join([state(p['state'])]+[n(p[k]) for k in ['x','y','rotation','poseFrame']])+'}' for p in poses)+'};']
            else:
                for j,vid in enumerate(IDS):
                    poses=[]
                    timeline=g.get('victimTimeline',[])
                    offset=g.get('victimOffsets',{}).get(vid,{})
                    for frame in range(m['totalFrames']+1):
                        previous=timeline[0] if timeline else None
                        following=timeline[-1] if timeline else None
                        for k in timeline:
                            if k['frame']<=frame: previous=k
                            if k['frame']>=frame:
                                following=k
                                break
                        phase=next((p for p in g.get('victimPhases',[]) if p['range']['from']<=frame<=p['range']['to']),{})
                        if previous:
                            alpha=max(0,min(1,(frame-previous['frame'])/max(1,following['frame']-previous['frame'])))
                            coords={key:previous[key]+(following[key]-previous[key])*alpha for key in ['victimAnchorX','victimAnchorY','victimRotation']}
                            same=previous['state']==following['state']
                            st=previous['state'] if same or frame<following['frame'] else following['state']
                            pose=int(previous['poseFrame']+(following['poseFrame']-previous['poseFrame'])*alpha+0.5) if same else previous['poseFrame']
                        else:
                            coords={key:phase.get(key,g[key]) for key in ['victimAnchorX','victimAnchorY','victimRotation']}
                            st=phase.get('state','grabbedFront' if frame<g['holdStartFrame'] else 'grabbedLifted')
                            pose=0
                        poses.append('GrabPoseDef{'+','.join([state(st),n(coords['victimAnchorX']+offset.get('anchorOffsetX',0)),n(coords['victimAnchorY']+offset.get('anchorOffsetY',0)),n(coords['victimRotation']+offset.get('rotationOffset',0)),n(pose)])+'}')
                    lines += [f'm.grabPoses[{j}] = {{'+','.join(poses)+'};']
        lines += ['d.canonicalMoves.push_back(std::move(m)); }']
    lines += ['break; }']
lines += ['}', '''int specials = 0;
for (const auto& m : d.canonicalMoves) {
 if (std::strcmp(m.id,"universalGrab")==0) d.universalGrab=m;
 else if (m.isSuper) d.superMove=m;
 else if (m.isSpecial) { if (specials++ == 0) d.special1=m; else d.special2=m; }
 else switch(m.anim) {
 case AnimId::StandingLight:d.standingLight=m;break;case AnimId::StandingHeavy:d.standingHeavy=m;break;
 case AnimId::ForwardLight:d.forwardLight=m;break;case AnimId::ForwardHeavy:d.forwardHeavy=m;break;
 case AnimId::CrouchLight:d.crouchLight=m;break;case AnimId::CrouchHeavy:d.crouchHeavy=m;break;
 case AnimId::AirLight:if(m.trajectory==0)d.airLight=m;break;case AnimId::AirHeavy:if(m.trajectory==0)d.airHeavy=m;break;
 default:break;
 }
}''', '}']
(OUT/'RDACanonicalData.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')

geo=D['presentation']['grabGeometry']
lines=['// Medidas originais da arte TS; gerado, não editar.']
for name,key in [('kGrabHands','hands')]:
    lines += ['static const float '+name+'[6][12][2] = {'+','.join('{'+','.join('{'+','.join(n(x) for x in p)+'}' for p in geo[key][fid])+'}' for fid in IDS)+'};']
lines += ['static const float kLiftHolds[6][8][2] = {'+','.join('{'+','.join('{'+','.join(n(x) for x in p[:2])+'}' for p in geo['landmarks'][fid]['lifted'])+'}' for fid in IDS)+'};']
lines += ['static const float kLiftTilts[6][8] = {'+','.join('{'+','.join(n(x) for x in geo['liftArt'][fid].get('tilts',[geo['liftArt'][fid]['tilt']*i/7 for i in range(8)]))+'}' for fid in IDS)+'};']
(OUT/'RDAGrabGeometry.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')

fnv=0xcbf29ce484222325
for byte in SOURCE.read_bytes(): fnv=((fnv^byte)*0x100000001b3)&0xffffffffffffffff
lines = ['// Gerado do motor TS; não editar expectativas.',f'constexpr uint64_t kFixtureFnv = 0x{fnv:016x}ULL;','struct ExpectedFighter { float x,y,vx,vy; int facing; FighterState state; int stateFrame,health,meter,roundWins; bool grounded; const char* move; std::array<float,13> mechanics; };','struct ReplayStep { int frame; CombatPhase phase; int phaseFrame; std::array<ExpectedFighter,2> fighters; int projectiles; const char* grab; };','struct ReplayInput { int h1,p1,h2,p2; };', 'void RunCanonicalReplay(const char*,const FighterDef&,const FighterDef&,GameMode,StageId,const ReplayStep*,const ReplayInput*,int);']
for i,s in enumerate(D['goldenScenarios']):
    lines += [f'const ReplayStep kReplay{i}[] = {{']
    for cp in s['checkpoints']:
        fighters=[]
        for f in cp['fighters']:
            fighters += ['ExpectedFighter{'+','.join([n(f[k]) for k in ['x','y','velocityX','velocityY','facing']]+[state(f['state'])]+[n(f[k]) for k in ['stateFrame','health','meter','roundWins','grounded']]+[q(f['moveId'] or ''),'{'+','.join(n(x) for x in f['mechanics'])+'}'])+'}']
        lines += ['{'+','.join([str(cp['frame']),'CombatPhase::'+cp['phase'][0].upper()+cp['phase'][1:],str(cp['phaseFrame']),'{'+','.join(fighters)+'}',str(cp['activeProjectiles']),q(cp['activeGrabMoveId'] or '')])+'},']
    lines += ['};',f'const ReplayInput kInput{i}[] = {{'+','.join('{'+','.join(str(inp[p][k]) for p,k in [('p1','heldMask'),('p1','pressedMask'),('p2','heldMask'),('p2','pressedMask')])+'}' for inp in s['inputs'])+'};']
lines += ['void TestCanonicalReplays() {']
for i,s in enumerate(D['goldenScenarios']):
    lines += ['RunCanonicalReplay('+','.join([q(s['id']),'Get'+ENUMS[IDS.index(s['playerOne'])]+'()','Get'+ENUMS[IDS.index(s['playerTwo'])]+'()','GameMode::Training' if s['mode']=='training' else 'GameMode::LocalVersus','StageId::'+['CaisDaCidade','CozinhaMacabra','Sitio'][['cais-da-cidade','cozinha-macabra','sitio'].index(s['arena'])],f'kReplay{i}',f'kInput{i}',str(s['totalSteps'])])+');']
lines += ['}']
(OUT/'RDACanonicalReplays.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print('Generated canonical catalog +',len(D['goldenScenarios']),'replays from SHA256',hashlib.sha256(SOURCE.read_bytes()).hexdigest())
