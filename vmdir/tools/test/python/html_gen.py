import re
import sys
import sqlite3
from optparse import OptionParser
from datetime import datetime

def db_dump_testruns():
    con = sqlite3.connect('testreport.db', isolation_level="IMMEDIATE")
    cur = con.cursor()
    cur.execute("SELECT * FROM testruns")
    return cur.fetchall()
    #testruns = []
    #for row in cur:
    #    testruns.append(row[0])
    #return testruns

def db_fetch_batch_stats(testrunId, batchNum):
    """Fetch data from Db, testrunId must be in the form of "YYYY-MM-DD HH:MM:SS.SSS"
    Return format is like : ds = [[ 63.39 ,'b', 'C', 300 ]...]
    """
    con = sqlite3.connect('testreport.db', isolation_level="IMMEDIATE")
    cur = con.cursor()
    t = (testrunId, batchNum )
    cur.execute("""SELECT duration, nodeName, opCode, count FROM worksets WHERE testRunId=? AND batchNum=?""", t)
    rows = cur.fetchall()
    return rows

def db_fetch_total_stats(testrunId):
    """Fetch data from Db, testrunId must be in the form of "YYYY-MM-DD HH:MM:SS.SSS"
    Return format is like : ds = [[ 63.39 ,'b', 'C', 300 ]...]
    """
    con = sqlite3.connect('testreport.db', isolation_level="IMMEDIATE")
    cur = con.cursor()
    t = (testrunId, )
    cur.execute("""SELECT duration, nodeName, opCode, count FROM worksets WHERE testRunId=?""", t)
    rows = cur.fetchall()
    return rows

def db_fetch_daily_stats(testrunId, reportDate):
    """Fetch data from Db, testrunId must be in the form of "YYYY-MM-DD HH:MM:SS.SSS"
    Return format is like : ds = [[ 63.39 ,'b', 'C', 300 ]...]
    """
    today = eval(reportDate + '000000.0')
    tomorrow = today + 1000000
    con = sqlite3.connect('testreport.db', isolation_level="IMMEDIATE")
    cur = con.cursor()
    t = (testrunId, today, tomorrow)
    cur.execute("""SELECT duration, nodeName, opCode, count FROM worksets WHERE testRunId=? AND endTime>?
                   and endTime<?""", t)
    rows = cur.fetchall()
    return rows

def gen_html(datasets, testrunInfos, nodeSum, opSum):
    """ds=dataset, in forms of ds = [[ 63.39 ,'b', 'C', 300 ], ...]"
    testrunInfo is an entire database record from table 'testruns'
    For example:
2013-01-10 10:56:42.568|3|sb-1564004|Linux|uat-1|1|{'a': '10.151.133.215', 'c': '10.151.128.63', 'b': '10.151.141.163'}|['a', 'b', 'c']|[['b', 'a'], ['c', 'a']]|[]|
   a
  / \ 
 /   \ 
b     c
    """
    _ops = ['C', 'R', 'U', 'D', 'All']
    _metrics = ['Count', 'Time', 'Tps']
    _suffix = { 'Count' : '', 'Time'  : 's', 'Tps'   : '/s' }
    _zero =   { 'Count' : 0,  'Time'  : 0.0, 'Tps'   : 0.0  }
    _adder = {}
    for metric in _metrics:
        _adder[metric] = eval(type(_zero[metric]).__name__ + '.__add__')

    _m = {
        'C':'Creates',
        'R':'Reads',
        'U':'Updates',
        'D':'Deletes',
        'All':'All ops',
        'Count':'Count',
        'Time':'Time(seconds)',
        'Tps':'Throughput(tps)'
    }

    rs = [] # perf reports, each entry generates 1 perf table
    timeStamp            = []
    numVMs               = []
    buildNumber          = []
    testOS               = []
    poolName             = []
    numBegin             = []
    vms                  = []
    nodeIds              = []
    initial_merge_seq    = []
    additional_merge_seq = []
    topology             = []
    for i, ti in enumerate(testrunInfos):
        ds = datasets[i]
        print str(ti[0])
        timeStamp.append(ti[0])
        numVMs.append(ti[1])
        buildNumber.append(ti[2])
        #testOS.append(ti[3])
        #poolName.append(ti[4])
        #numBegin.append(ti[5])
        #vms.append(eval(ti[6].upper()))
        #_nodeIds              = list(eval(ti[7].upper())).append('Sum')
        _nodeIds              = eval(ti[7].upper())
        _nodeIds.append('Sum')
        nodeIds.append(_nodeIds)
        #initial_merge_seq.append(eval(ti[8].upper()))
        #additional_merge_seq.append(eval(ti[9].upper()))
        #topology.append(ti[10].upper())

        _nodes = _nodeIds

        r = {}
        # Reset everything to zero
        for node in _nodes:
            r[node] = {}
            for op in _ops:
                r[node][_m[op]]={}
                for metric in _metrics:
                    r[node][_m[op]][_m[metric]]=_zero[metric]

        # starting calculation
        for line in ds:
            time, node, opcode, count = line[:]
            node = node.upper()
            r[node][_m[opcode]][_m['Count']] += count
            r[node][_m[opcode]][_m['Time']] += time
        # Compute 'All ops' for each node row, e.g., 'A' and 'B'
        for node in _nodes[:-1]:
            for metric in _metrics[:-1]: # The last one - Tps - will be calculated later
                r[node][_m[_ops[-1]]][_m[metric]] = reduce(_adder[metric], ( r[node][_m[op]][_m[metric]]
                                                                for op in _ops[:-1]), _zero[metric])
        # Compute 'Count' and 'Time' for row 'Sum'
        for op in _ops:
            for metric in _metrics[:-1]: # The last one - Tps - will be calculated later
                r[_nodes[-1]][_m[op]][_m[metric]] = reduce(_adder[metric], ( r[node][_m[op]][_m[metric]]
                                                                for node in _nodes[:-1]), _zero[metric])
        # Compute 'Tps' for every cell
        for node in _nodes:
            for op in _ops:
                if r[node][_m[op]][_m['Time']]:
                    r[node][_m[op]][_m['Tps']] = r[node][_m[op]][_m['Count']] / r[node][_m[op]][_m['Time']]

        # Add to perf reports
        rs.append(r)

    html = """<!DOCTYPE html>
    <html>
    <head>
        <title>Lotus Long-haul Stress Replication Test Report</title>
        <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/twitter-bootstrap/2.2.2/css/bootstrap.min.css" />
    </head>
    <body>
        <script src="http://code.jquery.com/jquery-latest.js"></script>
        <script src="https://netdna.bootstrapcdn.com/twitter-bootstrap/2.2.2/js/bootstrap.js"></script>
        <div class="container-fluid">
    """

    summary = """
        <h1>Summary</h1>
        <p class="text-error">Lotus long haul test is blocked.</p>
    """
    html += summary

    bugs = """
        <h3>Bugs</h3>
        <table class="table">
        <caption>Bug report</caption>
          <thead>
            <tr>
              <th>#</th>
              <th>Description</th>
              <th>Impact</th>
              <th>Date found/repro'ed</th>
              <th>Comments</th>
            </tr>
          </thead>
          <tbody>
            <tr class="error">
              <td><a href="http://bugzilla.eng.vmware.com/show_bug.cgi?id=991118">991118</a></td>
              <td>vdcmerge fail while trying to get kerberos master key</td>
              <td>This bug cause vdcmerge to fail<p class="text-error">This blocks Lotus long haul testing.</p></td>
              <td>2013-02-07</td>
              <td>This blocks cloudvm long haul test</td>
            </tr>
            <tr class="info">
              <td><a href="http://bugzilla.eng.vmware.com/show_bug.cgi?id=979340">979340</a></td>
              <td>vmdird: backend: MDB_PAGE_FULL error during entry deletion</td>
              <td>This bug cause some entries can't be deleted on the buggy VM node. Since entries can't be deleted, it causes long-haul stress test to fail. </td>
              <td>2013-01-11</td>
              <td><p class="text-success">We got a fix for this, and verified the fix is working.</p></td>
            </tr>
            <tr class="error">
              <td><a href="http://bugzilla.eng.vmware.com/show_bug.cgi?id=977224">977224</a></td>
              <td>ServerId conflicts on 6-node cloudvm replication</td>
              <td>This bug cause entries can't be created on the buggy VM node. Since entries can't be created, it also causes long-haul stress test to fail.</td>
              <td>2013-01-08</td>
              <td></td>
            </tr>
            <tr class="info">
              <td><a href="http://bugzilla.eng.vmware.com/show_bug.cgi?id=977249">977249</a></td>
              <td>Missing replication server nodes in 6-node cloudvm</td>
              <td>Root cause was that merge failed due to replication partner was down at the time of merge. Will watch out for repro</td>
              <td>2013-01-08</td>
              <td>Bug closed</td>
            </tr>
          </tbody>
        </table>
    """

    html += bugs
    html += """
        <h3>Perf stats</h3>
"""
    if not opSum:
        _ops.pop()
    for i, r in enumerate(rs):
        perftable = """
        <table class="table table-bordered">
            <caption>Perf summary on a """

        perftable += repr(numVMs[i])
        perftable += "-node replication cluster (since "
        perftable += timeStamp[i]
        perftable += " ) build="
        perftable += buildNumber[i]
        perftable += """</caption>
            <thead>
                <tr>
                    <th>Node</th>
                    <th>Metrix</th>
"""

        if not nodeSum:
            nodeIds[i].pop()

        for op in _ops:
            perftable += '                <th>%s</th>\n' % (_m[op])

        perftable += """            </tr>
            </thead>
            <tbody>
        """

        for node in nodeIds[i]:
            for i, metric in enumerate(_metrics):
                tablerow = "            <tr>\n"
                if i==0: # Only the first line need to span 3 rows
                    rowspan = len(_metrics)
                    tablerow += '              <td rowspan="%d">%s</td>\n' % (rowspan, node)
                tablerow += '              <td>%s</td>\n' % (_m[metric])
                for op in _ops:
                    if isinstance(r[node][_m[op]][_m[metric]], int):
                        tablerow += '              <td>%d%s</td>\n' % (r[node][_m[op]][_m[metric]], _suffix[metric])
                    elif isinstance(r[node][_m[op]][_m[metric]], float):
                        tablerow += '              <td>%.2f%s</td>\n' % (r[node][_m[op]][_m[metric]], _suffix[metric])
                tablerow += '            </tr>\n'
                perftable += tablerow
        perftable += """
            </tbody>
        </table>
        """

        html += perftable

    html += """ </div>
    </body>
    </html>
    """
    return html

def checkRequiredArguments(opts, parser):
    missing_options = []
    for option in parser.option_list:
        if re.match(r'^\[REQUIRED\]', option.help) and eval('opts.' + option.dest) == None:
            missing_options.extend(option._long_opts)
    if len(missing_options) > 0:
        parser.error('Missing REQUIRED parameters: ' + str(missing_options))

if __name__ == '__main__':
    today = format(datetime.now(), '%Y%m%d')
    parser = OptionParser()
    parser.add_option("-a","--a",dest="showData",default=False,action="store_true",help="Show dataset")
    parser.add_option("-b","--b",dest="batchNum",default=0,action="store",help="0-Inf")
    parser.add_option("-d","--d",dest="reportDate",default=today,action="store",help="YYYYMMDD")
    parser.add_option("-l","--l",dest="lastRuns",default=0,action="store",help="0-Inf: show # of last testRunIds info")
    parser.add_option("-n","--n",dest="nodeSum",default=False,action="store_true",help="Sum up ops on all NODES")
    parser.add_option("-p","--p",dest="opSum",default=False,action="store_true",help="Sum up All op TYPES")
    parser.add_option("-o","--o",dest="noHtml",default=False,action="store_true",help="Dry run, no html generation")
    parser.add_option("-t","--t",dest="testruns",action="store",help="[REQUIRED]'YYYY-MM-DD HH:MM:SS.SSS'")
    parser.add_option("-u","--u",dest="unique",default=False,action="store_true",help="Generate unique report file")
    parser.add_option("-y","--y",dest="reportType",type="choice",action="store",help="[REQUIRED]batch|daily|total",
                                              choices=['batch','daily','total',])
    (options, args) = parser.parse_args()
    db_testruns     = db_dump_testruns()
    if options.lastRuns:
        print 'Results are available for the following testruns:'
        print 'ID                        vmBuild#     OS    VM IPs               Comment'
        for t in db_testruns[0-int(options.lastRuns):]:
            print '"%s" %9s %7s%20s %s' % (t[0], t[2], t[3], '\n                                            '.join(
                                                       sorted(t[6].upper().replace('{',' ').rstrip('}').split(','))), t[11])
        sys.exit()
    if options.showData or not options.noHtml:
        checkRequiredArguments(options, parser)
    # Fetch data from Db and Get full testrun info
    testrunInfos = []
    datasets     = []
    db_testrun_Ids  = [ t[0] for t in db_testruns ]
    if options.testruns == "*":
        rept_testrun_Ids = db_testrun_Ids
    else:
        rept_testrun_Ids = [ t.lstrip(' ').rstrip(' ') for t in options.testruns.split(',') ]
    for tid in rept_testrun_Ids:
        if (options.reportType == 'daily'):
            ds = db_fetch_daily_stats(tid, options.reportDate)
        elif (options.reportType == 'batch'):
            ds = db_fetch_batch_stats(tid, options.batchNum)
        elif (options.reportType == 'total'):
            ds = db_fetch_total_stats(tid)
        datasets.append(ds)
        testrunInfos.append(db_testruns[db_testrun_Ids.index(tid)])
    # show test data if desired
    if options.showData:
        for ds in datasets:
            for d in ds:
                print str(d).replace(', u',', ').upper()
    # Generate html source code
    if not options.noHtml:
        html_src = gen_html(datasets, testrunInfos, options.nodeSum, options.opSum)
        # Dump html code to report file
        if not options.unique:
            fname = 'html/report-%s.html' % (options.reportDate)
        else:
            fname = 'html/report-%s.html' % (format(datetime.now(),'%Y%m%d%H%M%S.%f'))
        with open(fname, 'w') as f:
            f.write(html_src)
            print '%s generated successfully!' %(fname)
